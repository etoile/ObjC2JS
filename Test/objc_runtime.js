
function objc_msgSend(object, selector)
{
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
	// TODO: Other lookup failure mechanisms (Cocoa allows classes to add a
	// method when a missing one is required.
	// FIXME: throw some kind of exception
}

function objc_msgSendSuper(isClassMsg, cls, object, selector)
{
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


