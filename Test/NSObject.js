
C.NSAllocateObject = function(cls) {
	return cloneObject(cls.template);
};
C.NSDeallocateObject = function(obj) {}

OBJC.NSObject = new ObjCClass("NSObject", null)

// NSObject class methods
OBJC.NSObject.isa.methods["alloc"] = function(self, _cmd) { return C.NSAllocateObject(self); };
OBJC.NSObject.isa.methods["allocWithZone"] = function(self, _cmd) { return C.NSAllocateObject(self); };
OBJC.NSObject.isa.methods["new"] = function(self, _cmd) { return objc_msgSend(objc_msgSend(self, "alloc"), "init"); }


// NSObject instance methods

OBJC.NSObject.methods["copy"] = function(self, _cmd) { return objc_msgSend(self, "copyWithZone:", nil); };
OBJC.NSObject.methods["dealloc"] = function(self, _cmd) { C.NSDeallocateObject(self); }
OBJC.NSObject.methods["class"] = function(self, _cmd) { return self.isa; }

OBJC.NSObject.methods["retain"] = function(self, _cmd) { return self; };
OBJC.NSObject.methods["init"] = function(self, _cmd) { return self; };
OBJC.NSObject.methods["self"] = function(self, _cmd) { return self; };
OBJC.NSObject.methods["release"] = function(self, _cmd) {};
OBJC.NSObject.methods["description"] = function(self, _cmd) { return makeObjCString(self.isa.name);};
OBJC.NSObject.methods["alert"] = function(self, _cmd) { alert(objc_msgSend(self, "description")); };
