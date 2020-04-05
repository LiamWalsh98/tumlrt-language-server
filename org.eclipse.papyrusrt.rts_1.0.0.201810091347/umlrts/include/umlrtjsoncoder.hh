// umlrtjsoncoder.hh

#ifndef UMLRTJSONCODER_HH
#define UMLRTJSONCODER_HH

#include "document.h"
#include "writer.h"
#include "stringbuffer.h"
using namespace rapidjson;

struct UMLRTSignal;
struct UMLRTCommsPort;
struct UMLRTObject_class;

class UMLRTJSONCoder
{
public:
	static bool fromJSON( const char * json, const UMLRTCommsPort *& port, UMLRTSignal & signal );
	static char * toJSON( const UMLRTSignal & signal );

private:
	static void encodeParam( Document & document, Value & paramObj, const UMLRTObject_class * desc, void * data );
	static bool decodeParam( const Value & param, const UMLRTObject_class * desc, uint8_t * buffer );
};

#endif // UMLRTJSONCODER_HH
