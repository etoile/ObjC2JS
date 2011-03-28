C = new Object();
OBJC = new Object();

function cloneObject(obj)
{
	var clone = function() {};
	clone.prototype = obj;
	return new clone();
}


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
				case 16:  typedBuffer = Int16Array(buffer);
				case 32:  typedBuffer = Int32Array(buffer);
			}
		}
		else
		{
			switch (size)
			{
				case 8:  typedBuffer = Uint8Array(buffer);
				case 16:  typedBuffer = Uint16Array(buffer);
				case 32:  typedBuffer = Uint32Array(buffer);
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
			typedBuffer = Float64Array(buffer);
		}
	}
	typedBuffer.buffer = buffer;
	return typedBuffer;
}

/**
 * Constructs a C string from a JavaScript string
 */
function makeCString(str)
{
	var buffer = ArrayBuffer(str.length);
	var c_str = Int8Array(buffer);
	c_str.buffer = buffer;
	for (var i=0 ; i<buffer.length ; i++)
	{
		c_str[i] = str.charCodeAt(i);
	}
	return c_str;
}

function makeObjCString(str)
{
	str.isa = OBJC.NSConstantString;
}

C.malloc = function(size) { return new ArrayBuffer(size); }
C.calloc = function(size, n) { return new ArrayBuffer(size * n); }
C.jsalert = function(x) { alert(x); };
