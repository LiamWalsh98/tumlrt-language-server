// umlrtmqttservice.hh

#ifndef UMLRTMQTTSERVICE_HH
#define UMLRTMQTTSERVICE_HH

#include "umlrtbasicthread.hh"
#include "umlrtqueue.hh"
#include "umlrtqueueelement.hh"
#include "umlrtcommsport.hh"
#include "umlrthashmap.hh"

typedef void* MQTTClient;
typedef struct MQTTClient_message_t MQTTClient_message;

class UMLRTMQTTService : UMLRTBasicThread
{
public:
	UMLRTMQTTService ( ) : UMLRTBasicThread("UMLRTMQTTService")
	{
	}

    class Message : public UMLRTQueueElement {
	public:
    	const UMLRTCommsPort * port;
		char * topic;
		char * payload;
		int length;
    };

    void spawn ( );

    static void connect ( const UMLRTCommsPort * port, const char * mqttHost, int mqttPort,
    		const char * username = NULL, const char* password = NULL );
    static void disconnect ( const UMLRTCommsPort * port );
    static void subscribe( const UMLRTCommsPort * port, const char * topic );
    static void publish ( const UMLRTCommsPort * port, const char * topic , char * payload, int length );
    static void publish ( const UMLRTCommsPort * port, const char * topic , const char * msg );

private:
    virtual void * run ( void * args );

    static char * generateClientID ( const int len );
    static int messageReceived ( void * context, char * topicName, int topicLen, MQTTClient_message * message );
    static MQTTClient * getClient ( const UMLRTCommsPort * port );
    static UMLRTHashMap * getPortClientMap ( );

    static UMLRTHashMap * portClientMap;
    static UMLRTQueue inQueue;
};

#endif // UMLRTMQTTSERVICE_HH
