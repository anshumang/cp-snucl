/*****************************************************************************/
/*                                                                           */
/* Copyright (c) 2011-2012 Seoul National University.                        */
/* All rights reserved.                                                      */
/*                                                                           */
/* Redistribution and use in source and binary forms, with or without        */
/* modification, are permitted provided that the following conditions        */
/* are met:                                                                  */
/*   1. Redistributions of source code must retain the above copyright       */
/*      notice, this list of conditions and the following disclaimer.        */
/*   2. Redistributions in binary form must reproduce the above copyright    */
/*      notice, this list of conditions and the following disclaimer in the  */
/*      documentation and/or other materials provided with the distribution. */
/*   3. Neither the name of Seoul National University nor the names of its   */
/*      contributors may be used to endorse or promote products derived      */
/*      from this software without specific prior written permission.        */
/*                                                                           */
/* THIS SOFTWARE IS PROVIDED BY SEOUL NATIONAL UNIVERSITY "AS IS" AND ANY    */
/* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED */
/* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE    */
/* DISCLAIMED. IN NO EVENT SHALL SEOUL NATIONAL UNIVERSITY BE LIABLE FOR ANY */
/* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL        */
/* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS   */
/* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)     */
/* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,       */
/* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  */
/* ANY WAY OUT OF THE USE OF THIS  SOFTWARE, EVEN IF ADVISED OF THE          */
/* POSSIBILITY OF SUCH DAMAGE.                                               */
/*                                                                           */
/* Contact information:                                                      */
/*   Center for Manycore Programming                                         */
/*   School of Computer Science and Engineering                              */
/*   Seoul National University, Seoul 151-744, Korea                         */
/*   http://aces.snu.ac.kr                                                   */
/*                                                                           */
/* Contributors:                                                             */
/*   Jungwon Kim, Sangmin Seo, Jun Lee, Jeongho Nah, Gangwon Jo, Jaejin Lee  */
/*                                                                           */
/*****************************************************************************/

#include <arch/fusion/Page.h>
#include <malloc.h>
#include <sys/mman.h>

SNUCL_DEBUG_HEADER("Page");

Page::Page(void* addr, size_t len) {
  this->addr = addr;
  this->len = len;
}

Page::~Page() {

}

bool Page::Contains(void* p) {
  size_t _p = (size_t) p;
  size_t _addr = (size_t) addr;

  SNUCL_DEBUG("P[%p] ADDR[%p ~ 0x%lx]", p, addr, _addr + len);

  return _p >= _addr && _p < (_addr + len);
}

PageRepository* PageRepository::s_instance = new PageRepository();

PageRepository::PageRepository() {
  RegisterHandler();
}

PageRepository::~PageRepository() {

}

void PageRepository::SavePages(void* addr) {
  for (map<Page*, CLCommand*>::iterator it = pages.begin(); it != pages.end(); ++it) {
    Page* page = it->first;
    if (page->Contains(addr)) {
      CLCommand* command = it->second;
      CLMem* mem_dst = command->mem_dst;
      size_t off_dst = command->off_dst;
      size_t size = command->cb;
      void* ptr = command->ptr;

      size_t _addr = (size_t) addr;
      size_t _ptr = (size_t) ptr;

      if (!mem_dst->space_host) {
        SNUCL_CHECK();
        mem_dst->space_host = memalign(PAGESIZE, mem_dst->size);
      }

      if (_addr >= _ptr && _addr < _ptr + size) { // Modifying the buffer
        SNUCL_CHECK();
        memcpy((void*) ((size_t) mem_dst->space_host + off_dst), ptr, size);
        if (mprotect(page->addr, page->len, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
          SNUCL_ERROR("FAILED TO MPROTECT(%p, %lu, %d)", page->addr, page->len, PROT_READ | PROT_WRITE | PROT_EXEC);
        }
      } else { // Modifying other data in the same page
        SNUCL_DEBUG("ADDR[%p] PTR[%p]", addr, ptr);
        if (addr < ptr) {
          SNUCL_CHECK();
          void* npage = NPAGE(ptr);
          size_t remain_size = (size_t) npage - (size_t) ptr;
          memcpy((void*) ((size_t) mem_dst->space_host + off_dst), ptr, remain_size);
          SNUCL_DEBUG("NPAGE[%p] REMAIN_SIZE[%lx]", npage, remain_size);
        } else {
          SNUCL_CHECK();
          void* cpage = PAGE((size_t) ptr + size);
          size_t remain_size = (size_t) ptr + size - (size_t) cpage;
          memcpy((void*) ((size_t) mem_dst->space_host + off_dst + size - remain_size), (void*) ((size_t) ptr + size - remain_size), remain_size);
        }
        if (mprotect(PAGE(addr), PAGESIZE, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
          SNUCL_ERROR("FAILED TO MPROTECT(%p, %lu, %d)", page->addr, page->len, PROT_READ | PROT_WRITE | PROT_EXEC);
        }
      }
    }
  }
}

void PageRepository::RecordPage(CLCommand* command) {
  size_t size = command->cb;
  void* ptr = command->ptr;

  void* page_start = PAGE(ptr);
  void* page_end = NPAGE((size_t) ptr + size);
  size_t len = (size_t) page_end - (size_t) page_start;
  SNUCL_DEBUG("RECORD_PAGE [%p ~ %p] [0x%lx]", page_start, page_end, len);

  if (mprotect(page_start, len, PROT_READ) == -1) {
    SNUCL_ERROR("FAILED TO MPROTECT(%p, %lu, %d)", page_start, len, PROT_READ);
  }

  Page *page = new Page(page_start, len);
  pages[page] = command;
}

int PageRepository::GetPages(void* addr, vector<Page*>* ret) {
  int total = 0;
  for (map<Page*, CLCommand*>::iterator it = pages.begin(); it != pages.end(); ++it) {
    Page* page = it->first;
    if (page->Contains(addr)) {
      if (ret) ret->push_back(page);
      total++;
    }
  }
  return total;
}

CLCommand* PageRepository::GetCommand(Page* page) {
  if (pages.count(page) == 0) return NULL;
  return pages[page];
}

struct sigaction oldact;

void PageRepository::RegisterHandler() {
  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_SIGINFO;
  act.sa_sigaction = PageRepository::handle_SIGSEGV;
  sigaction(SIGSEGV, &act, &oldact);
}

void PageRepository::handle_SIGSEGV(int signum, siginfo_t* si, void* ctx) {
  SNUCL_CHECK();
  PageRepository::instance()->SavePages(si->si_addr);
}

