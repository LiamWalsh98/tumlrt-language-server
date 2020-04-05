// umlrtmqttproxycapsule.cc

#include "umlrtmqttproxycapsule.hh"

#include "basedebug.hh"
#include "basefatal.hh"
#include "umlrtcommsportrole.hh"
#include "umlrtcommsportfarend.hh"
#include "umlrtmessage.hh"
#include "umlrtmqttprotocol.hh"
#include "umlrtslot.hh"
#include <cstddef>
#include "umlrtcapsuleclass.hh"
#include "umlrtframeservice.hh"
#include "umlrtjsoncoder.hh"

class UMLRTRtsInterface;
struct UMLRTCommsPort;

Capsule_MQTTProxyCapsule::Capsule_MQTTProxyCapsule( const UMLRTCapsuleClass * cd, UMLRTSlot * st, const UMLRTCommsPort * * border, const UMLRTCommsPort * * internal, bool isStat )
: UMLRTCapsule( NULL, cd, st, border, internal, isStat )
, currentState( SPECIAL_INTERNAL_STATE_UNVISITED )
, mqtt( internalPorts[internalport_mqtt] )
, host( NULL )
, port( 0 )
, username( NULL )
, password( NULL )
, topic( NULL )
, subscriptions( NULL )
{
    stateNames[Connecting] = "Connecting";
    stateNames[Error] = "Error";
    stateNames[Waiting] = "Waiting";
    stateNames[SPECIAL_INTERNAL_STATE_TOP] = "<top>";
    stateNames[SPECIAL_INTERNAL_STATE_UNVISITED] = "<uninitialized>";

    if(st->numPorts != 1)
    	FATAL("MQTTProxyCapsule must have exactly one border port");
}


void Capsule_MQTTProxyCapsule::inject( const UMLRTMessage & message )
{
    msg = &message;
    switch( currentState )
    {
    case Connecting:
        currentState = state_____Connecting( &message );
        break;
    case Error:
        currentState = state_____Error( &message );
        break;
    case Waiting:
        currentState = state_____Waiting( &message );
        break;
    default:
        break;
    }
}

void Capsule_MQTTProxyCapsule::initialize( const UMLRTMessage & message )
{
    msg = &message;
    actionchain_____Initial( &message );
    currentState = Connecting;
}

const char * Capsule_MQTTProxyCapsule::getCurrentStateString() const
{
    return stateNames[currentState];
}

void Capsule_MQTTProxyCapsule::update_state( Capsule_MQTTProxyCapsule::State newState )
{
    currentState = newState;
}

void Capsule_MQTTProxyCapsule::entryaction_____Connecting( const UMLRTMessage * msg )
{
    #define rtdata ( (void *)msg->getParam( 0 ) )

	mqtt.connect(host, port, username, password);

    #undef rtdata
}

void Capsule_MQTTProxyCapsule::transitionaction_____conn_error( const UMLRTMessage * msg )
{
    #define errno ( *(int *)msg->getParam( 0 ) )
    #define rtdata ( (int *)msg->getParam( 0 ) )

    FATAL("MQTT connection error (%d)", errno);

    #undef rtdata
    #undef errno
}

void Capsule_MQTTProxyCapsule::transitionaction_____connected( const UMLRTMessage * msg )
{
    #define rtdata ( (void *)msg->getParam( 0 ) )

	if(subscriptions != NULL) {
		char* topic = strtok(subscriptions, ";");
		while(topic != NULL) {
			mqtt.subscribe(topic);
			topic = strtok(NULL, ";");
		}
	}

	borderPorts[0]->recall(-1, true, false, -1);

    #undef rtdata
}

void Capsule_MQTTProxyCapsule::transitionaction_____msg_recv( const UMLRTMessage * msg )
{
    #define topic ( *(const char * *)msg->getParam( 0 ) )
    #define payload ( *(const char * *)msg->getParam( 1 ) )
    #define length ( *(int *)msg->getParam( 2 ) )
    #define rtdata ( (const char * *)msg->getParam( 0 ) )

	//TODO: multiple border ports + multiple farEnds
	UMLRTSignal signal;
	if(UMLRTJSONCoder::fromJSON(payload, borderPorts[0], signal))
		slot->controller->deliver(borderPorts[0]->farEnds[0].port, signal, borderPorts[0]->roleIndex);

    #undef rtdata
    #undef length
    #undef payload
    #undef topic
}

void Capsule_MQTTProxyCapsule::transitionaction_____recv_error( const UMLRTMessage * msg )
{
    #define errno ( *(int *)msg->getParam( 0 ) )
    #define rtdata ( (int *)msg->getParam( 0 ) )

	FATAL("MQTT error (%d)", errno);

    #undef rtdata
    #undef errno
}

void Capsule_MQTTProxyCapsule::actionchain_____Initial( const UMLRTMessage * msg )
{
    update_state( Connecting );
    entryaction_____Connecting( msg );
}

void Capsule_MQTTProxyCapsule::actionchain_____conn_error( const UMLRTMessage * msg )
{
    update_state( SPECIAL_INTERNAL_STATE_TOP );
    transitionaction_____conn_error( msg );
    update_state( Error );
}

void Capsule_MQTTProxyCapsule::actionchain_____connected( const UMLRTMessage * msg )
{
    update_state( SPECIAL_INTERNAL_STATE_TOP );
    transitionaction_____connected( msg );
    update_state( Waiting );
}

void Capsule_MQTTProxyCapsule::actionchain_____msg_recv( const UMLRTMessage * msg )
{
    update_state( SPECIAL_INTERNAL_STATE_TOP );
    transitionaction_____msg_recv( msg );
    update_state( Waiting );
}

void Capsule_MQTTProxyCapsule::actionchain_____recv_error( const UMLRTMessage * msg )
{
    update_state( SPECIAL_INTERNAL_STATE_TOP );
    transitionaction_____recv_error( msg );
    update_state( Error );
}

Capsule_MQTTProxyCapsule::State Capsule_MQTTProxyCapsule::state_____Connecting( const UMLRTMessage * msg )
{
    switch( msg->destPort->role()->id )
    {
    case port_mqtt:
        switch( msg->getSignalId() )
        {
        case UMLRTMQTTProtocol::signal_connected:
            actionchain_____connected( msg );
            return Waiting;
        case UMLRTMQTTProtocol::signal_error:
            actionchain_____conn_error( msg );
            return Error;
        default:
            this->unexpectedMessage();
            break;
        }
        return currentState;
    default:
    	msg->defer();
        break;
    }
    return currentState;
}

Capsule_MQTTProxyCapsule::State Capsule_MQTTProxyCapsule::state_____Error( const UMLRTMessage * msg )
{
    switch( msg->destPort->role()->id )
    {
    default:
        this->unexpectedMessage();
        break;
    }
    return currentState;
}

Capsule_MQTTProxyCapsule::State Capsule_MQTTProxyCapsule::state_____Waiting( const UMLRTMessage * msg )
{
    switch( msg->destPort->role()->id )
    {
    case port_mqtt:
        switch( msg->getSignalId() )
        {
        case UMLRTMQTTProtocol::signal_error:
            actionchain_____recv_error( msg );
            return Error;
        case UMLRTMQTTProtocol::signal_received:
            actionchain_____msg_recv( msg );
            return Waiting;
        default:
            this->unexpectedMessage();
            break;
        }
        return currentState;
    default:
    	proxyMessage( msg );
        break;
    }
    return currentState;
}

void Capsule_MQTTProxyCapsule::proxyMessage( const UMLRTMessage * msg )
{
	char* json = UMLRTJSONCoder::toJSON(msg->signal);
	if(json != NULL) {
		if(topic != NULL)
			mqtt.publish(topic, json);
		else
			mqtt.publish(slot->capsuleClass->name, json);
		free(json);
	}
}

void Capsule_MQTTProxyCapsule::bindPort( bool isBorder, int portId, int index )
{
}

void Capsule_MQTTProxyCapsule::unbindPort( bool isBorder, int portId, int index )
{
}

static const UMLRTCommsPortRole portroles_internal[] = 
{
    {
        Capsule_MQTTProxyCapsule::port_mqtt,
        "MQTT",
        "mqtt",
        "",
        0,
        false,
        false,
        false,
        false,
        true,
        false,
        false
    },
    {
        Capsule_MQTTProxyCapsule::port_log,
        "Log",
        "log",
        "",
        0,
        false,
        false,
        false,
        false,
        true,
        false,
        false
    }
};

static void instantiate_MQTTProxyCapsule( const UMLRTRtsInterface * rts, UMLRTSlot * slot, const UMLRTCommsPort * * borderPorts )
{
    const UMLRTCommsPort * * internalPorts = UMLRTFrameService::createInternalPorts( slot, &MQTTProxyCapsule );
    slot->capsule = new Capsule_MQTTProxyCapsule( &MQTTProxyCapsule, slot, borderPorts, internalPorts, false );
}

const UMLRTCapsuleClass MQTTProxyCapsule =
{
    "MQTTProxyCapsule",
    NULL,
	instantiate_MQTTProxyCapsule,
    0,
    NULL,
    0,
    NULL,
    2,
    portroles_internal
};

