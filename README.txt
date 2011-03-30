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

- C primitive types are represented by JavaScript primitives.  All C number
  types are JavaScript numbers, which are double-precision floating-point
  values.  Cast operations are implemented by truncating the value to fit in
  the destination type.

- C structured types are implemented using WebGL ArrayBuffer objects.  If you
  allocate a structure or an array, you get an ArrayBuffer.

- Pointers are implemented as AddressOf objects.  These implement pointer
  arithmetic and dereferencing.  When you take the address of an object, you
  get a new AddressOf object, whose pointee field is set to the object whose
  address you took.  The result of a pointer arithmetic expression is a new
  AddressOf object, as long as the pointee is a valid address

- Casting pointers to integers gives a unique 32-bit integer value.  There is
  no mechanism for casting integers to pointers, and it is not possible to
  implement one without JavaScript gaining support for weak references.

- If you attempt to store a pointer in memory buffer (i.e. a C array or
  structure), then the underlying buffer object has the pointer stored as a
  property and the integer value written into the buffer at that address.  This
  allows you to store a pointer in a structure or union (for example) and then
  access its value as an integer.  If you attempt to cast from this integer
  back to a pointer, then you get undefined behaviour.  If you simply
  dereference the pointer as a pointer type, however, then you will load back
  the pointer value stored in the buffer.

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
