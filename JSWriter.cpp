//===- JavaScriptGeneratorNames.cpp ---------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Example clang plugin which simply prints the names of all the top-level decls
// in the input file.
//
//===----------------------------------------------------------------------===//

#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/AST.h"
#include "clang/AST/RecordLayout.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/Support/raw_ostream.h"

#include <stdio.h>

using namespace clang;

namespace {



class JSWriter : public ASTConsumer,
                 public RecursiveASTVisitor<JSWriter> {
  llvm::raw_fd_ostream OS;
  ASTContext *Ctx;

public:
  JSWriter(CompilerInstance &CI, llvm::StringRef InFile) : ASTConsumer(),
    RecursiveASTVisitor<JSWriter>(), OS(STDOUT_FILENO, false, true) {
  }
  virtual void HandleTranslationUnit(ASTContext &C) {
    Ctx = &C;
    TraverseDecl(C.getTranslationUnitDecl());
    Ctx = 0;
  }
  bool TraverseFunctionDecl(FunctionDecl *D) {
    if (!D->getBody()) { return true; }
    // FIXME: Static functions need moving to a file namespace
    OS << "C." << D->getNameAsString() << " = function(";
    for (FunctionDecl::param_iterator i=D->param_begin(),e=D->param_end() ;
        i!=e ; i++) {
      OS << (*i)->getName();
      if (i+1 != e) {
        OS << ',';
      }
    }
    OS << ") ";
    TraverseStmt(D->getBody());
    return true;
  }
  const char *JSPrimitiveForType(QualType Ty) {
    if (Ty->isIntegerType()) {
      if (Ty->isUnsignedIntegerType()) {
        switch (Ctx->getTypeSize(Ty))
        {
          case 8: return "Uint8";
          case 16: return "Uint16";
          case 32: return "Uint32";
          case 64: //FIXME: Emit error, no 64-bit int types in JS
          default: return 0;
        }
      } else {
        switch (Ctx->getTypeSize(Ty))
        {
          case 8: return "Int8";
          case 16: return "Int16";
          case 32: return "Int32";
          case 64: //FIXME: Emit error, no 64-bit int types in JS
          default: return 0;
        }
      }
    } else if (Ty->isFloatingType()) {
        switch (Ctx->getTypeSize(Ty))
        {
          case 32: return "Float32";
          case 64: return "Float64";
          default: return 0;
        }
    } else if (Ty->isAnyPointerType()) {
      return "Pointer";
    }
    return 0;
  }
  void EmitVarDecl(VarDecl *VD) {
    QualType Ty = VD->getType();
    OS << VD->getName();
    if (Stmt *init = VD->getInit()) {
      OS << " = ";
      TraverseStmt(init);
      // If it's a composite type, we need to create some memory to store it.
      // If there's an init list then this is done when emitting the code for
      // that
    } else if (Ty->isStructureType() || Ty->isArrayType()) {
      OS << " = new ArrayBuffer(" << Ctx->getTypeSize(Ty) / 8 << ')';
    }

  }
  bool TraverseVarDecl(VarDecl *VD) {
    OS << "var ";
    EmitVarDecl(VD);
    OS << ";\n";
    return true;
  }
  bool TraverseDeclStmt(DeclStmt *D) {
    OS << "var ";
    for (DeclStmt::decl_iterator i=D->decl_begin(),e=D->decl_end() ; i!=e ; ++i){
      EmitVarDecl(cast<VarDecl>(*i));
      if (i+1 != e)
        OS << ", ";
    }
    return true;
  }

  void EmitInitList(InitListExpr *E, uint64_t baseOffset) {
    const RecordType *RT = cast<RecordType>(cast<ElaboratedType>(E->getType())->getNamedType());
    const RecordDecl *RD = RT->getDecl();
    const ASTRecordLayout &Layout = Ctx->getASTRecordLayout(RD);
    Expr **Init = E->getInits();
    // This is almost certainly broken for array initialisers
    for (RecordDecl::field_iterator i=RD->field_begin(),e=RD->field_end()
        ; i!=e ; ++i) {
      // TODO: Bitfields (yuck!)
      uint64_t offset = baseOffset + Layout.getFieldOffset((*i)->getFieldIndex()) / 8;
      QualType FieldTy = i->getType();
      if (FieldTy->isArithmeticType()) {
        OS << "$tmp.set" << JSPrimitiveForType(FieldTy) << '(' << offset <<  ", ";
        TraverseStmt(*Init);
        OS << ");";
      } else if (FieldTy->isAnyPointerType()) {
        OS << "$tmp.setPointer(" << offset <<  ", ";
        TraverseStmt(*Init);
        OS << ");";
      } else if (InitListExpr *SubList = dyn_cast<InitListExpr>(*Init)) {
        EmitInitList(SubList, offset);
      }
      Init++;
    }
  }

  bool TraverseInitListExpr(InitListExpr *E) {
    OS << "(function () { var $tmp = new ArrayBuffer("
       << Ctx->getTypeSize(E->getType()) / 8 << ");";
    EmitInitList(E, 0);
    OS << " return $tmp; })()";
    return true;
  }

  bool VisitStringLiteral(StringLiteral *SL) {
    OS << "makeCString(\"" << SL->getString() << "\")";
    return true;
  }
  bool TraverseObjCStringLiteral(ObjCStringLiteral *L) {
    StringLiteral *SL = L->getString();
    OS << "makeObjCString(\"" << SL->getString() << "\")";
    return true;
  }

  bool VisitIntegerLiteral(IntegerLiteral *IL) {
    OS << IL->getValue();
    return true;
  }
  bool VisitDeclRefExpr(DeclRefExpr *E) {
    OS << E->getDecl()->getName();
    return true;
  }
  bool TraverseBinAssign(BinaryOperator *Op) {
    if (isa<DeclRefExpr>(Op->getLHS()))
      return TraverseBinOp(Op);
    OS << '(';
    TraverseStmt(Op->getLHS());
    // FIXME: Structure assignments (memcpy call)
    // Store the value at address 0 in the LHS.
    OS << ").set" << JSPrimitiveForType(Op->getLHS()->getType()) << "(0, ";
    TraverseStmt(Op->getRHS());
    OS << ')';
    return true;
  }
  // These binary ops are the same in C and JS, so they can just be emitted as-is.
  bool TraverseBinAdd(BinaryOperator *Op) { return TraverseBinOp(Op); }
  bool TraverseBinAnd(BinaryOperator *Op) { return TraverseBinOp(Op); }
  bool TraverseBinComma(BinaryOperator *Op) { return TraverseBinOp(Op); }
  // FIXME: add Math.abs() around integer division
  bool TraverseBinDiv(BinaryOperator *Op) { return TraverseBinOp(Op); }
  bool TraverseBinEQ(BinaryOperator *Op) { return TraverseBinOp(Op); }
  bool TraverseBinGE(BinaryOperator *Op) { return TraverseBinOp(Op); }
  bool TraverseBinGT(BinaryOperator *Op) { return TraverseBinOp(Op); }
  bool TraverseBinLE(BinaryOperator *Op) { return TraverseBinOp(Op); }
  bool TraverseBinLT(BinaryOperator *Op) { return TraverseBinOp(Op); }
  bool TraverseBinLand(BinaryOperator *Op) { return TraverseBinOp(Op); }
  bool TraverseBinLor(BinaryOperator *Op) { return TraverseBinOp(Op); }
  bool TraverseBinMul(BinaryOperator *Op) { return TraverseBinOp(Op); }
  bool TraverseBinNE(BinaryOperator *Op) { return TraverseBinOp(Op); }
  bool TraverseBinOr(BinaryOperator *Op) { return TraverseBinOp(Op); }
  bool TraverseBinRem(BinaryOperator *Op) { return TraverseBinOp(Op); }
  bool TraverseBinShl(BinaryOperator *Op) { return TraverseBinOp(Op); }
  bool TraverseBinShr(BinaryOperator *Op) { return TraverseBinOp(Op); }
  bool TraverseBinSub(BinaryOperator *Op) { return TraverseBinOp(Op); }
  bool TraverseBinXor(BinaryOperator *Op) { return TraverseBinOp(Op); }

  bool TraverseBinOp(BinaryOperator *Op) {
    TraverseStmt(Op->getLHS());
    OS << Op->getOpcodeStr();
    TraverseStmt(Op->getRHS());
    return true;
  }
  bool TraversePrefixOp(UnaryOperator *Op) {
    OS << Op->getOpcodeStr(Op->getOpcode());
    TraverseStmt(Op->getSubExpr());
    return true;
  }
  bool TraversePostfixOp(UnaryOperator *Op) {
    TraverseStmt(Op->getSubExpr());
    OS << Op->getOpcodeStr(Op->getOpcode());
    return true;
  }
  bool TraverseUnaryPostInc(UnaryOperator *Op) { return TraversePostfixOp(Op); }
  bool TraverseUnaryPostDec(UnaryOperator *Op) { return TraversePostfixOp(Op); }
  bool TraverseUnaryPreInc(UnaryOperator *Op) { return TraversePrefixOp(Op); }
  bool TraverseUnaryPreDec(UnaryOperator *Op) { return TraversePrefixOp(Op); }
  bool TraverseUnaryPlus(UnaryOperator *Op) { return TraversePrefixOp(Op); }
  bool TraverseUnaryMinus(UnaryOperator *Op) { return TraversePrefixOp(Op); }
  bool TraverseUnaryNot(UnaryOperator *Op) { return TraversePrefixOp(Op); }
  bool TraverseUnaryLNot(UnaryOperator *Op) { return TraversePrefixOp(Op); }

  /**
   * Take the address of an object by creating an array containing that object.
   * Modifications to the object will then modify the original object.  
   */
  bool TraverseUnaryAddrOf(UnaryOperator *Op) {
    OS << "new AddressOf(";
    TraverseStmt(Op->getSubExpr());
    OS << ')';
    return true;
  }
  bool TraverseUnary(UnaryOperator *Op) {
    OS << '(';
    TraverseStmt(Op->getSubExpr());
    OS << ").dereference()";
    return true;
  }

  bool TypesEquivalent(QualType a, QualType b) {
    if (Ctx->hasSameUnqualifiedType(a, b))
      return true;
    if (a->isPointerType() && b->isPointerType())
      return TypesEquivalent(a->getPointeeType(), b->getPointeeType());
    return false;
  }

  bool TraverseCastExpr(CastExpr *E) {
    // Most C cast expressions can be completely ignored in JS - they will
    // happen by magic at run time.
    Expr *Body = E->getSubExpr();
    switch (E->getCastKind()) {
      default:
        TraverseStmt(Body);
        break;
      case CK_ArrayToPointerDecay:
        OS << "(new AddressOf(";
        TraverseStmt(Body);
        OS << "))";
        break;
      case CK_LValueToRValue:
        TraverseStmt(Body);
        // If this is an l-value to r-value cast, then it's really a load
        // expression.  Do that, except for direct references to variables,
        // which we can read directly.
        if (!isa<DeclRefExpr>(Body) && JSPrimitiveForType(E->getType())) {
          OS << ".get" << JSPrimitiveForType(E->getType()) << "(0)";
        }
        break;
      case CK_FloatingToIntegral:
        // Integers and floating point values in JS are the same, but we should
        // drop the fractional part.
        OS << "Math.abs(";
        TraverseStmt(Body);
        OS << ')';
        break;
      case CK_AnyPointerToObjCPointerCast:
          E->dump();
        llvm::errs() << "Casts to Objective-C object types are not supported.";
        return false;
      case CK_BitCast:
          TraverseStmt(Body);
          return true;
        QualType ToTy = E->getType().getUnqualifiedType();
        QualType FromTy = Body->getType().getUnqualifiedType();
        if ((ToTy == FromTy) ||
            (ToTy->isObjCObjectPointerType() &&
             FromTy->isObjCObjectPointerType())||
            TypesEquivalent(ToTy, FromTy)) {
          TraverseStmt(Body);
          return true;
        }
        if (E->getType()->isPointerType() && Body->getType()->isPointerType()) {
          QualType Ty = E->getType()->getPointeeType();
          QualType OldTy = Body->getType()->getPointeeType();

          // We can cast int and float pointers to and from void easily
          if (Ty->isVoidType() && (OldTy->isArithmeticType()
                                   || OldTy->isVoidType())) {
            TraverseStmt(Body);
            return true;
          }
          if (OldTy->isVoidType() && (Ty->isArithmeticType()
                                      || Ty->isVoidType())) {
            TraverseStmt(Body);
            return true;
          }
          if (OldTy->isArithmeticType() && Ty->isArithmeticType()) {
            OS << '(';
            TraverseStmt(Body);
            OS << ").pointerCastTo(";
            OS << ", ";
            OS << (Ty->isIntegerType() ? "true, " : "false, ");
            OS << (Ty->isUnsignedIntegerType() ? "false, " : "true, ");
            OS << Ctx->getTypeSize(Ty);
            OS << ')';
            return true;
          }
          E->dump();
          /*
           * Enum stuff:
          Expr::EvalResult Result;
          E->Evaluate(Result, CGF.getContext());
          OS << Result.Val.getInt();
          */
        }
        else {
          E->dump();
          return false;
        }
    }
    return true;
  }
  bool TraverseCStyleCastExpr(CStyleCastExpr *E) {
    return TraverseCastExpr(E);
  }
  bool TraverseImplicitCastExpr(ImplicitCastExpr *E) {
    return TraverseCastExpr(E);
  }
  bool TraverseArraySubscriptExpr(ArraySubscriptExpr *E) {
    TraverseStmt(E->getBase());
    // FIXME: If it's a constant expression, evaluate it here!
    OS << ".pointerAdd(" << Ctx->getTypeSize(E->getType())/8 << " * ";
    TraverseStmt(E->getIdx());
    OS << ").dereference()";
    return true;
  }

  bool TraverseUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr *E) {
    // FIXME: VLA sizes
    Expr::EvalResult Result;
    E->Evaluate(Result, *Ctx);
    OS << Result.Val.getInt();;
    return true;
  }

  bool TraverseCompoundStmt(CompoundStmt *S) {
    OS <<"{\n";
    for (CompoundStmt::body_iterator i=S->body_begin(),e=S->body_end() ; i!=e ;
        ++i){
      TraverseStmt(*i);
      OS << ";\n";
    }
    OS << "}\n";
    return true;
  }
  bool TraverseParenExpr(ParenExpr *E) {
    OS << "(";
    TraverseStmt(E->getSubExpr());
    OS << ")";
    return true;
  }
  bool TraverseForStmt(ForStmt *S) {
    OS << "for (";
    TraverseStmt(S->getInit());
    OS << " ; ";
    TraverseStmt(S->getCond());
    OS << " ; ";
    TraverseStmt(S->getInc());
    OS << ")\n";
    TraverseStmt(S->getBody());
    return true;
  }
  bool TraverseWhileStmt(WhileStmt *S) {
    OS << "while(";
    TraverseStmt(S->getCond());
    OS << ')';
    TraverseStmt(S->getBody());
    return true;
  }
  bool TraverseDoStmt(DoStmt *S) {
    OS << "do ";
    TraverseStmt(S->getBody());
    OS << "while(";
    TraverseStmt(S->getCond());
    OS << ')';
    return true;
  }

  bool TraverseCallExpr(CallExpr *E) {
    // If this is a direct call, then we call the function in the C 'namespace'
    // FIXME: Move static functions into a per-file namespace!
    if (FunctionDecl *FD = E->getDirectCallee()) {
      OS << "C." << FD->getName();
    } else {
      TraverseStmt(E->getCallee());
    }
    OS << '(';
    for (CallExpr::arg_iterator i=E->arg_begin(),e=E->arg_end() ; i!=e ; ++i) {
      if ((*i)->getType()->isStructureType()){
        OS << "cloneObject(";
        TraverseStmt(*i);
        OS << ")";
      } else {
        TraverseStmt(*i);
      }
      if (i+1 != e)
        OS << ", ";
    }
    OS << ')';
    return true;
  }
  bool TraverseObjCMessageExpr(ObjCMessageExpr *E) {
    // FIXME: super
    OS << "objc_msgSend(";
    if (E->isClassMessage())
      OS << "OBJC." <<
        E->getClassReceiver()->getAs<ObjCObjectType>()->getInterface()->getName();
    else
      TraverseStmt(E->getInstanceReceiver());
    OS << ", \"" << E->getSelector().getAsString() << '"';
    for (ObjCMessageExpr::arg_iterator i=E->arg_begin(),e=E->arg_end() ; i!=e ; ++i) {
      TraverseStmt(*i);
      if (i+1 != e)
        OS << ", ";
    }
    OS << ')';
    return true;
  }
  bool TraverseObjCMethodDecl(ObjCMethodDecl *OMD) {
    Stmt *body = OMD->getBody();
    // Declaration, ignore it.
    if (0 == body) return true;

    OS << "OBJC." << OMD->getClassInterface()->getName();
    if (OMD->isClassMethod())
      OS << ".isa";
    OS << ".methods[\"" << OMD->getSelector().getAsString();
    OS << "\"] = function(self, _cmd";
    for (ObjCMethodDecl::param_iterator i=OMD->param_begin(),e=OMD->param_end()
        ; i!=e ; ++i) {
      OS << ", ";
      TraverseDecl(*i);
    }
    OS << ")";
    TraverseStmt(body);
    OS << ";\n";
    return true;
  }
  bool VisitObjCPropertyImplDecl(ObjCPropertyImplDecl *D) {
    // Skip dynamic properties
    if (D->getPropertyImplementation() == ObjCPropertyImplDecl::Dynamic) { return true;}

    ObjCPropertyDecl *OPD = D->getPropertyDecl();
    ObjCIvarDecl *Ivar = D->getPropertyIvarDecl();
    ObjCContainerDecl *cls = cast<ObjCContainerDecl>(Ivar->getDeclContext());
    OS << "OBJC." << cls->getName();
    OS << ".methods[\"" << OPD->getGetterName().getAsString();
    OS << "\"] = function(self, _cmd) { return self." << Ivar->getName();
    OS << "; };\n";
    if (!OPD->isReadOnly()) {
      OS << "OBJC." << cls->getName();
      OS << ".methods[\"" << OPD->getSetterName().getAsString();
      OS << "\"] = function(self, _cmd, newVal) { self." << Ivar->getName();
      OS << "= newVal; };\n";
    }
    return true;
  }
  const char* OpForAssignOp(BinaryOperatorKind Op) {
    switch (Op) {
      case BO_AddAssign: return "+";
      case BO_AndAssign: return "&";
      case BO_DivAssign: return "/";
      case BO_OrAssign: return "|";
      case BO_RemAssign: return "%";
      case BO_ShlAssign: return "<<";
      case BO_ShrAssign: return ">>";
      case BO_SubAssign: return "-";
      case BO_XorAssign: return "^";
      default: return "?";
    }
  }
  bool TraverseCompoundAssignOperator(CompoundAssignOperator *Op) {
    // FIXME: This is probably wrong, because we might be evaluating side
    // effects twice
    Expr *LHS = Op->getLHS();
    Expr *RHS = Op->getRHS();
    if (ObjCPropertyRefExpr *E = dyn_cast<ObjCPropertyRefExpr>(LHS)) {
      OS << "objc_msgSend(";
      TraverseStmt(E->getBase());
      OS << ", \"" << E->getSetterSelector().getAsString() << "\", ";
      OS << "objc_msgSend(";
      TraverseStmt(E->getBase());
      OS << ", \"" << E->getGetterSelector().getAsString() << "\")";
      OS << OpForAssignOp(Op->getOpcode());
      TraverseStmt(RHS);
      OS << ')';
    } else {
      TraverseStmt(LHS);
      OS << Op->getOpcodeStr();
      TraverseStmt(RHS);
    }
    return true;
  }
  bool TraverseBinAndAssign(CompoundAssignOperator *Op) {
    return TraverseCompoundAssignOperator(Op);
  }
  bool TraverseBinAddAssign(CompoundAssignOperator *Op) {
    return TraverseCompoundAssignOperator(Op);
  }
  bool TraverseBinDivAssign(CompoundAssignOperator *Op) {
    return TraverseCompoundAssignOperator(Op);
  }
  bool TraverseBinOrAddAssign(CompoundAssignOperator *Op) {
    return TraverseCompoundAssignOperator(Op);
  }
  bool TraverseBinRemAssign(CompoundAssignOperator *Op) {
    return TraverseCompoundAssignOperator(Op);
  }
  bool TraverseBinShlAssign(CompoundAssignOperator *Op) {
    return TraverseCompoundAssignOperator(Op);
  }
  bool TraverseBinShrAssign(CompoundAssignOperator *Op) {
    return TraverseCompoundAssignOperator(Op);
  }
  bool TraverseBinSubAssign(CompoundAssignOperator *Op) {
    return TraverseCompoundAssignOperator(Op);
  }
  bool TraverseBinXorAssign(CompoundAssignOperator *Op) {
    return TraverseCompoundAssignOperator(Op);
  }

  void WriteMemberExprBase(MemberExpr *E) {
    // If this is an arrow expression then we need to dereference it by getting
    // the pointer at address 0 in this memory view.  The getPointer() function
    // will translate this offset back into the last value written.
    OS << '(';
    TraverseStmt(E->getBase());
    OS << ')';
    if (E->isArrow())
      OS << ".dereference()";
  }

  bool TraverseMemberExpr(MemberExpr *E) {
    Expr *Base = E->getBase();
    QualType BaseTy = Base->getType();
    const RecordType *StructTy = BaseTy->getAsStructureType();
    if (NULL == StructTy && BaseTy->isPointerType())
      StructTy = BaseTy->getPointeeType()->getAsStructureType();
    const ASTRecordLayout &Layout = Ctx->getASTRecordLayout(StructTy->getDecl());
    // FIXME: Bitfields (yuck!)
    uint64_t offset =
      Layout.getFieldOffset(cast<FieldDecl>(E->getMemberDecl())->getFieldIndex()) / 8;
    QualType Ty = E->getType();
    // If this is an l-value expression then we just do the pointer arithmetic.
    if (E->isLValue()) {
      OS << '(';
      WriteMemberExprBase(E);
      OS << ").pointerAdd(" << offset << ')';
      return true;
    }

    // If the result is a pointer, then we need to read a pointer from the
    // buffer's pointer store, which is parallel to its data store.
    if (Ty->isPointerType()) {
      OS << "getPointer(";
      WriteMemberExprBase(E);
      OS << offset << ')';
      return true;
    }
    WriteMemberExprBase(E);
    // We now have two cases: A nested structure or a primitive.  If it's a
    // structure, then we do some pointer arithmetic and return a new view on
    // this buffer.  If it's a primitive, then we also do a load of that field
    if (Ty->isArithmeticType()) {
      OS << ".get" << JSPrimitiveForType(Ty) << '(' << offset << ')';
      return true;
    }
    return true;
  }

  bool TraverseObjCPropertyRefExpr(ObjCPropertyRefExpr *E) {
    OS << "objc_msgSend(";
    TraverseStmt(E->getBase());
    OS << ", \"" << E->getGetterSelector().getAsString() << "\")";
    return true;
  }
  bool VisitObjCImplDecl(ObjCImplDecl *D) {
    OS << "OBJC." << D->getName() << " = new ObjCClass(\"" << D->getName() << "\", ";
    if (ObjCInterfaceDecl *Super = D->getClassInterface()->getSuperClass())
      OS << '"' << Super->getName() << '"';
    else
      OS << "null";
    OS << ")\n";
    return true;
  }
  bool VisitObjCIvarDecl(ObjCIvarDecl *D) {
    ObjCContainerDecl *cls = cast<ObjCContainerDecl>(D->getDeclContext());
    OS << "OBJC." << cls->getName();
    OS << ".ivars.push(\"" << D->getName() << "\");\n";
    OS << "OBJC." << cls->getName();
    OS << ".template[\"" << D->getName() << "\"] = null;\n";
    return true;
  }
  bool TraverseReturnStmt(ReturnStmt *S) {
    OS << "return ";
    if (Expr *E = S->getRetValue()) {
      TraverseStmt(E);
    }
    return true;
  }
};

class JavaScriptGeneratorAction : public PluginASTAction {
protected:
  ASTConsumer *CreateASTConsumer(CompilerInstance &CI, llvm::StringRef inFile) {
    return new JSWriter(CI, inFile);
  }

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string>& args) {
    return true;
  }
};

}

static FrontendPluginRegistry::Add<JavaScriptGeneratorAction>
X("-emit-js", "Writes the compilation unit out as JavaScript code");
