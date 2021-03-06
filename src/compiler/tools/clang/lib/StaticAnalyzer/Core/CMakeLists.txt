set(LLVM_LINK_COMPONENTS support)

set(LLVM_USED_LIBS clangBasic clangLex clangAST clangFrontend clangRewrite)

add_clang_library(clangStaticAnalyzerCore
  AggExprVisitor.cpp
  AnalysisManager.cpp
  BasicConstraintManager.cpp
  BasicStore.cpp
  BasicValueFactory.cpp
  BugReporter.cpp
  BugReporterVisitors.cpp
  CFRefCount.cpp
  CheckerContext.cpp
  CheckerHelpers.cpp
  CheckerManager.cpp
  Environment.cpp
  ExplodedGraph.cpp
  ExprEngine.cpp
  FlatStore.cpp
  BlockCounter.cpp
  CXXExprEngine.cpp
  CoreEngine.cpp
  GRState.cpp
  HTMLDiagnostics.cpp
  MemRegion.cpp
  ObjCMessage.cpp
  PathDiagnostic.cpp
  PlistDiagnostics.cpp
  RangeConstraintManager.cpp
  RegionStore.cpp
  SimpleConstraintManager.cpp
  SimpleSValBuilder.cpp
  Store.cpp
  SValBuilder.cpp
  SVals.cpp
  SymbolManager.cpp
  TextPathDiagnostics.cpp
  )

add_dependencies(clangStaticAnalyzerCore ClangAttrClasses ClangAttrList ClangDeclNodes
                 ClangStmtNodes)
