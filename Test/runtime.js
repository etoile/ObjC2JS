C = new Object();
OBJC = new Object();

function cloneObject(obj)
{
	var clone = function() {};
	clone.prototype = obj;
	return new clone();
}

C.NSAllocateObject = function(cls) {
	return cloneObject(cls.template);
};
C.NSDeallocateObject = function(obj) {}

C.malloc = function(size) { return new ArrayBuffer(size); }
C.calloc = function(size, n) { return new ArrayBuffer(size * n); }

function pointerCastTo(obj, integer, signed, size)
{
	var buffer = obj;
	if (obj.constructor != ArrayBuffer)
	{
		buffer = obj.buffer;
	}
	var typedBuffer;
	if (integer)
	{
		if (signed)
		{
			switch (size)
			{
				case 8:  typedBuffer = Int8Array(buffer);
				case 16:  typedBuffer = Int8Array(buffer);
				case 32:  typedBuffer = Int8Array(buffer);
			}
		}
		else
		{
			switch (size)
			{
				case 8:  typedBuffer = Uint8Array(buffer);
				case 16:  typedBuffer = Uint8Array(buffer);
				case 32:  typedBuffer = Uint8Array(buffer);
			}
		}
	}
	else
	{
		if (size == 32)
		{
			typedBuffer = Float32Array(buffer);
		}
		else
		{
			typedBuffer = Float32Array(buffer);
		}
	}
	typedBuffer.buffer = buffer;
	return typedBuffer;
}

function objc_msgSend(object, selector) {
	if (!object) return null;
	var method = object.isa.methods[selector];
	if (method)
		return method.apply(object, arguments);
	if (object.isa.methods.forwardingTargetForSelector)
	{
		arguments[0] = object.isa.methods.forwardingTargetForSelector(object, "forwardingTargetForSelector");
		return objc_msgSend.apply(this, arguments);
	}
	if (object.isa.methods.forwardInvocation)
	{
		// FIXME: Make the third argument an NSInvocation, not an array
		return object.isa.methods.forwardInvocation(object, "forwardInvocation", arguments);
	}
	// FIXME: throw some kind of exception
}

function objc_msgSendSuper(isClassMsg, cls, object, selector) {
	var method;
	if (isClassMsg)
	{
		method = OBJC[cls].isa.isa.methods[selector];
	}
	else
	{
		method = OBJC[cls].isa.methods[selector];
	}
	// Discard the first two elements
	arguments.shift();
	arguments.shift();
	method.apply(object, arguments);
}

function objc_initClass(cls, superClass)
{
	cls.template = new Object();
	cls.template.isa = cls;
	cls.methods = new Object();
	cls.ivars = new Array();
	cls.protocols = new Array();
	cls.properties = new Array();
	cls.name = name;
	if (superClass)
	{
		//cls.methods.__proto__ = superClass.methods;
		//var clone = function() {};
		//clone.prototype = superClass.methods;
		//cls.methods = new clone();
		cls.methods = cloneObject(superClass.methods);
		cls.superclass = superClass;
	}
}

function ObjCClass(name, superclassName)
{
	var superClass = OBJC[superclassName];
	objc_initClass(this, superClass, name);
	this.isa = new Object();
	objc_initClass(this.isa, superClass ? superClass.isa : null, name);
}

OBJC.NSObject = new ObjCClass("NSObject", null)
// NSObject class methods
OBJC.NSObject.isa.methods["alloc"] = function(self, _cmd) { return C.NSAllocateObject(self); };
OBJC.NSObject.isa.methods["new"] = function(self, _cmd) { return objc_msgSend(objc_msgSend(self, "alloc"), "init"); }
// NSObject instance methods
OBJC.NSObject.methods["retain"] = function(self, _cmd) { return self; };
OBJC.NSObject.methods["init"] = function(self, _cmd) { return self; };
OBJC.NSObject.methods["self"] = function(self, _cmd) { return self; };
OBJC.NSObject.methods["release"] = function(self, _cmd) {};
OBJC.NSObject.methods["alert"] = function(self, _cmd) { alert(self); };
