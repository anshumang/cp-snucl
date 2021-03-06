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
/*   Sangmin Seo, Jungwon Kim, Jun Lee, Jeongho Nah, Gangwon Jo,             */
/*   and Jaejin Lee                                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* This file is based on the SNU-SAMSUNG OpenCL Compiler and is distributed  */
/* under the same license, GNU General Public License.                       */
/* See LICENSE.SNU-SAMSUNG_OpenCL_C_Compiler.TXT for details.                */
/*****************************************************************************/

#ifndef SNUCLC_STMTSIMPLIFIER_H
#define SNUCLC_STMTSIMPLIFIER_H

#include "StmtTransformer.h"
#include "CLExpressions.h"
#include "TypeDefs.h"

namespace clang {
namespace snuclc {

/// StmtSimplifier
/// - Split DeclStmt that has Init the expression into the declaration and
///   the definition.
/// - Split DeclStmt that has multiple declarations.
/// - Divide a comma expression into multiple single expressions.
class StmtSimplifier : public StmtTransformer {
  ASTContext    &ASTCtx;
  CLExpressions &CLExprs;
  llvm::raw_ostream &OS;

  StmtVector *CurBody;
  ExprVector PostExprVec;

public:
  StmtSimplifier(ASTContext &C, CLExpressions &Exprs)
    : ASTCtx(C), CLExprs(Exprs), OS(llvm::errs()) {
    CurBody = NULL;
  }

  void Transform(FunctionDecl *FD);

  CompoundStmt *TransformRawCompoundStmt(CompoundStmt *Node);
  Stmt *TransformRawIfStmt(IfStmt *If);

  Stmt *VisitNullStmt(NullStmt *Node);
  Stmt *VisitCompoundStmt(CompoundStmt *Node);
  Stmt *VisitLabelStmt(LabelStmt *Node);
  Stmt *VisitIfStmt(IfStmt *If);
  Stmt *VisitSwitchStmt(SwitchStmt *Node);
  Stmt *VisitWhileStmt(WhileStmt *Node);
  Stmt *VisitDoStmt(DoStmt *Node);
  Stmt *VisitForStmt(ForStmt *Node);
  Stmt *VisitGotoStmt(GotoStmt *Node);
  Stmt *VisitIndirectGotoStmt(IndirectGotoStmt *Node);
  Stmt *VisitContinueStmt(ContinueStmt *Node);
  Stmt *VisitBreakStmt(BreakStmt *Node);
  Stmt *VisitReturnStmt(ReturnStmt *Node);
  Stmt *VisitDeclStmt(DeclStmt *Node);
  Stmt *VisitCaseStmt(CaseStmt *Node);
  Stmt *VisitDefaultStmt(DefaultStmt *Node);

  Stmt *VisitPredefinedExpr(PredefinedExpr *Node);
  Stmt *VisitDeclRefExpr(DeclRefExpr *Node);
  Stmt *VisitIntegerLiteral(IntegerLiteral *Node);
  Stmt *VisitFloatingLiteral(FloatingLiteral *Node);
  Stmt *VisitImaginaryLiteral(ImaginaryLiteral *Node);
  Stmt *VisitStringLiteral(StringLiteral *Str);
  Stmt *VisitCharacterLiteral(CharacterLiteral *Node);
  Stmt *VisitParenExpr(ParenExpr *Node);
  Stmt *VisitUnaryOperator(UnaryOperator *Node);
  Stmt *VisitOffsetOfExpr(OffsetOfExpr *Node);
  Stmt *VisitSizeOfAlignOfExpr(SizeOfAlignOfExpr *Node);
  Stmt *VisitVecStepExpr(VecStepExpr *Node);
  Stmt *VisitArraySubscriptExpr(ArraySubscriptExpr *Node);
  Stmt *VisitCallExpr(CallExpr *Call);
  Stmt *VisitMemberExpr(MemberExpr *Node);
  Stmt *VisitBinaryOperator(BinaryOperator *Node);
  Stmt *VisitCompoundAssignOperator(CompoundAssignOperator *Node);
  Stmt *VisitConditionalOperator(ConditionalOperator *Node);
  Stmt *VisitImplicitCastExpr(ImplicitCastExpr *Node);
  Stmt *VisitCStyleCastExpr(CStyleCastExpr *Node);
  Stmt *VisitCompoundLiteralExpr(CompoundLiteralExpr *Node);
  Stmt *VisitExtVectorElementExpr(ExtVectorElementExpr *Node);
  Stmt *VisitInitListExpr(InitListExpr* Node);
  Stmt *VisitDesignatedInitExpr(DesignatedInitExpr *Node);
  Stmt *VisitParenListExpr(ParenListExpr* Node);
  Stmt *VisitVAArgExpr(VAArgExpr *Node);

private:
  Expr *VisitExprInStmt(Expr *E);
  Expr *VisitExprInAssignExpr(Expr *E);
  Expr *VisitTopLevelUnaryOperator(UnaryOperator *Node);
  void MakeArrayInitExprs(VarDecl *VD, Expr *Init, StmtVector &Defs);

  CompoundStmt *ConvertToCompoundStmt(Stmt *S);
}; //end class StmtSimplifier

} //end namespace snuclc
} //end namespace clang

#endif //SNUCLC_STMTSIMPLIFIER_H

