
// Stub implementations: should be replaced by the GNUstep ones!
OBJC.NSString = new ObjCClass("NSString", "NSObject")
OBJC.NSMutableString = new ObjCClass("NSMutableString", "NSString")


OBJC.JSString = new ObjCClass("JSString", "NSString")
OBJC.JSString.methods["length"] = function(self, _cmd) { return self.length; };
OBJC.JSString.methods["characterAt:"] = function(self, _cmd, i) { return self[i]; };

OBJC.NSConstantString = new ObjCClass("NSConstantString", "JSString")
