C = new Object();
OBJC = new Object();
CStatics = new Object();

function cloneObject(obj)
{
	var clone = function() {};
	clone.prototype = obj;
	return new clone();
}


/**
 * We can't generate pointers for Objective-C objects, but sometimes 
 */
var nextPointer = 1;

Object.prototype.getPointerValue = function()
{
	if (!this.__PointerValue)
	{
		this.__PointerValue = nextPointer++;
	}
	return this.__PointerValue;
}
/**
 * We add accessors to both ArrayBuffer and TypedArray that allow us to get any
 * value type that we want from an offset.
 */
ArrayBuffer.prototype.getInt8 = function(offset)
{
	return (new Int8Array(this))[offset];
}

ArrayBuffer.prototype.getInt16 = function(offset)
{
	return (new Int16Array(this))[offset/2];
}

ArrayBuffer.prototype.getInt32 = function(offset)
{
	return (new Int32Array(this))[offset / 4];
}

ArrayBuffer.prototype.getUint8 = function(offset)
{
	return (new Uint8Array(this))[offset];
}

ArrayBuffer.prototype.getUint16 = function(offset)
{
	return (new Uint16Array(this))[offset / 2];
}

ArrayBuffer.prototype.getUint32 = function(offset)
{
	return (new Uint32Array(this))[offset / 4];
}

ArrayBuffer.prototype.getFloat32 = function(offset)
{
	return (new Float32Array(this))[offset / 4];
}

ArrayBuffer.prototype.getFloat64 = function(offset)
{
	return (new Float64Array(this))[offset / 8];
}

ArrayBuffer.prototype.setInt8 = function(offset, obj)
{
	(new Int8Array(this))[offset] = obj;
}

ArrayBuffer.prototype.setInt16 = function(offset, obj)
{
	(new Int16Array(this))[offset/2] = obj;
}

ArrayBuffer.prototype.setInt32 = function(offset, obj)
{
	(new Int32Array(this))[offset/4] = obj;
}

ArrayBuffer.prototype.setUint8 = function(offset, obj)
{
	(new Uint8Array(this))[offset] = obj;
}

ArrayBuffer.prototype.setUint16 = function(offset, obj)
{
	(new Uint16Array(this))[offset/2] = obj;
}

ArrayBuffer.prototype.setUint32 = function(offset, obj)
{
	(new Uint32Array(this))[offset/4] = obj;
}

ArrayBuffer.prototype.setFloat32 = function(offset, obj)
{
	(new Float32Array(this))[offset/4] = obj;
}

ArrayBuffer.prototype.setFloat64 = function(offset, obj)
{
	(new Float64Array(this))[offset/8] = obj;
}


ArrayBuffer.prototype.toString = function()
{
	switch (this.byteLength)
	{
		default:
			return "[" + this.byteLength + " bytes]";
		case 1:
			return "'" + String.fromCharCode(this.getInt8(0)) + "'";
		case 2:
			return this.getInt16(0).toString();
		case 4:
			return this.getInt16(0).toString();
	}
}




Int8Array.prototype.getInt8 = function(offset)
{
	return this[offset];
}

Int8Array.prototype.getInt16 = function(offset)
{
	return this.buffer.getInt16(offset + this.byteOffset);
}

Int8Array.prototype.getInt32 = function(offset)
{
	return this.buffer.getInt32(offset + this.byteOffset);
}

Int8Array.prototype.getUint8 = function(offset)
{
	return this.buffer.getUint8(offset + this.byteOffset);
}

Int8Array.prototype.getUint16 = function(offset)
{
	return this.buffer.getUint16(offset + this.byteOffset);
}

Int8Array.prototype.getUint32 = function(offset)
{
	return this.buffer.getUint32(offset + this.byteOffset);
}

Int8Array.prototype.getFloat32 = function(offset)
{
	return this.buffer.getFloat32(offset + this.byteOffset);
}

Int8Array.prototype.getFloat64 = function(offset)
{
	return this.buffer.getFloat64(offset + this.byteOffset);
}

Int8Array.prototype.setInt8 = function(offset, obj)
{
	this[offset] = obj;
}

Int8Array.prototype.setInt16 = function(offset, obj)
{
	this.buffer.setInt16(offset + this.byteOffset, obj);
}

Int8Array.prototype.setInt32 = function(offset, obj)
{
	this.buffer.setInt32(offset + this.byteOffset, obj);
}

Int8Array.prototype.setUint8 = function(offset, obj)
{
	this.buffer.setUint8(offset + this.byteOffset, obj);
}

Int8Array.prototype.setUint16 = function(offset, obj)
{
	this.buffer.setUint16(offset + this.byteOffset, obj);
}

Int8Array.prototype.setUint32 = function(offset, obj)
{
	this.buffer.setUint32(offset + this.byteOffset, obj);
}

Int8Array.prototype.setFloat32 = function(offset, obj)
{
	this.buffer.setFloat32(offset + this.byteOffset, obj);
}

Int8Array.prototype.setFloat64 = function(offset, obj)
{
	this.buffer.setFloat64(offset + this.byteOffset, obj);
}


/**
 * Adds a pointer accessor to all objects
 */
Object.prototype.setPointer = function(offset, object)
{
	this[offset] = object;
}

ArrayBuffer.prototype.getPointer = function(offset)
{
	return this.pointers[offset];
}
ArrayBuffer.prototype.setPointer = function(offset, object)
{
	var buffer=this;
	// Store a fake pointer value, just in case someone wants to do some
	// pointer comparisons
	this.setInt32(offset, object.getPointerValue());
	// Store the real object as a property of the buffer, so we can retrieve it
	// again later
	if (!this.pointers)
	{
		this.pointers = new Object();
	}
	this.pointers[offset] = object;
}

Int8Array.prototype.setPointer = function(offset, object)
{
	var buffer = this.buffer;
	if (this.byteOffset)
	{
		offset += this.byteOffset;
	}
	buffer.setPointer(offset, object);
}

Object.prototype.getPointer = function(offset)
{
	return this[offset];
}
/**
 * Retrieves an Objective-C object value stored in a structure / array.  The
 * actual value stored is going to be a fake value, but when we cast out we
 * want to retrieve the original pointer.
 */
Int8Array.prototype.getPointer = function getPointer(offset)
{
	return this.buffer.getPointer(offset + this.byteOffset);
}

ArrayBuffer.prototype.pointerAdd = function(offset)
{
	var view = new Int8Array(this, offset);
	view.buffer = this;
	return view;
}
Int8Array.prototype.pointerAdd = function(offset)
{
	var view = new Int8Array(this.buffer, this.byteOffset + offset);
	view.buffer = this.buffer;
	return view;
}

function OutOfRangeAddress(offset)
{
	this.byteOffset = offset;
}

function AddressOf(obj)
{
	this.pointee = obj;
}
AddressOf.prototype.pointerAdd = function(offset)
{
	return new AddressOf(this.pointee.pointerAdd(offset));
}

AddressOf.prototype.dereference = function()
{
	return this.pointee;
}
AddressOf.prototype.getPointer= function(offset)
{
	// FIXME: error if offset != 0
	return this.pointee;
}

AddressOf.prototype.copy = function()
{
	var n = new AddressOf(this.pointee);
}

AddressOf.prototype.dereference = function()
{
	// This may be another AddressOf object.
	return this.pointee;
}

AddressOf.prototype.toString = function()
{
	return "&(" + this.pointee + ')';
}

/**
 * Constructs a C string from a JavaScript string
 */
function makeCString(str)
{
	var buffer = new ArrayBuffer(str.length);
	var c_str = new Int8Array(buffer);
	c_str.buffer = buffer;
	for (var i=0 ; i<str.length ; i++)
	{
		c_str.buffer.setInt8(i, str.charCodeAt(i));
	}
	return c_str;
}

function makeObjCString(str)
{
	str.isa = OBJC.NSConstantString;
	return str;
}

C.malloc = function(size) { return new AddressOf(new ArrayBuffer(size)); }
C.calloc = function(size, n) { return new AddressOf(new ArrayBuffer(size * n)); }

C.memcpy = function(dest, src, size)
{
	var inbuffer = src[0];
	var outbufer = dest[0];
	if (inbuffer.constructor != ArrayBuffer)
	{
		inbuffer = inbuffer.buffer;
	}
	if (outbuffer.constructor != ArrayBuffer)
	{
		outbuffer = inbuffer.buffer;
	}
	inbuffer = new Int8Array(inbuffer);
	outbuffer = new Int8Array(inbuffer);
	// FIXME: Copy pointer values as well!
	for (var i=0 ; i<size ; i++)
	{
		// TODO: This would be faster copying a Uint32 at a time...
		outbuffer.setUint8(i, inbuffer.getUint8(i));
	}
	// FIXME: Copy pointer values too.
}

C.jsalert = function(x) { alert(x); };
