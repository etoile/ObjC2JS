Objective-C to JavaScript Converter
===================================

This is a clang plugin that traverses the AST and emits JavaScript code
corresponding to an Objective-C compilation unit, along with a JavaScript
Objective-C runtime and set of core classes.  The goal of this project is to
allow Objective-C applications to be compiled to JavaScript and run unmodified.

In practice, this goal is unreachable, however we should be able to define a
safe subset of Objective-C that works both as native code and when run in a
JavaScript VM.

Current Status
--------------

This code is VERY work-in-progress.  If it works at all, then you should be
surprised!  Not everything in the How It Works section of this document is
actually implemented!

Compiling
---------

Unfortunately, the clang build system does not currently make any sensible
provision for building plugins outside of the clang tree.  You must, therefore,
check out or copy this directory inside the clang source tree.

Once this is done, it can be built by running GNU make (either make or gmake,
depending on your platform).

How It Works (in theory)
------------------------

The [Objective-]C type system doesn't really play nicely with the JavaScript,
but the compiler and runtime try to work around this, performing the following
mappings:

- C structures are mapped to JavaScript objects, so can be referenced by name
  in any context.  Casts of pointers to C structures are effectively null
  operations in JavaScript, so you can cast a structure pointer to another that
  has the same fields, but not to one that has an equivalent layout but
  different field names.  Eventually, this restriction can be eliminated by
  providing order in which the two structure fields are defined to a cast function.

- Pointers to C primitive value types (int, float, double, and so on) are
  implemented using the WebGL array extensions.  A void* pointer is a
  JavaScript ArrayBuffer, which is a class wrapping a block of untyped memory.
  Pointer arithmetic generates new ArrayBufferView, giving a different view of
  the same memory.  When you cast to a concrete pointer type, such as int*, you
  get TypedArray, which allows the memory to be interpreted as an array of
  primitive types.  **NOTE:** Casting pointer to complex types (structures or
  arrays, or Objective-C objects) is not supported.

- All pointers are represented by arrays (either ArrayBuffers or Arrays
  depending on how they were constructed).  This allows, for example, functions
  that return values via pointers, because the array object will be aliased by
  the caller and the callee and the caller can simply extract the element that
  is inserted.

- Objective-C objects are JavaScript objects, with an isa field set to a class.
  The classes are all added to a global OBJC object, allowing class lookup to
  work without seeing JavaScript objects.  Objective-C methods are JavaScript
  functions, with self and _cmd parameters.  These are looked up by looking at
  the class's methods field, which is an object whose prototype is the
  superclass's methods field.  This means that inheritance works as expected.
  Methods are called by the objc_msgSend() function, which performs the lookup
  and then calls the function.  This also allows -forwardInvocation: and
  friends to work.

- C functions are translated to JavaScript functions and added to fields of the
  C global object.  
