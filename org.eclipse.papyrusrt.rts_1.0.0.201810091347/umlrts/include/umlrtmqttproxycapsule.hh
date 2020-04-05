// umlrtmqttproxycapsule.hh

#ifndef UMLRTMQTTPROXYCAPSULE_HH
#define UMLRTMQTTPROXYCAPSULE_HH

#include "umlrtcapsule.hh"
#include "umlrtcapsuleclass.hh"
#include "umlrtlogprotocol.hh"
#include "umlrtmessage.hh"
#include "umlrtmqttprotocol.hh"
struct UMLRTCommsPort;
struct UMLRTSlot;

class Capsule_MQTTProxyCapsule : public UMLRTCapsule
{
public:
    Capsule_MQTTProxyCapsule( const UMLRTCapsuleClass * cd, UMLRTSlot * st, const UMLRTCommsPort * * border, const UMLRTCommsPort * * internal, bool isStat );
    enum PortId
    {
        port_log,
        port_mqtt
    };
    void inject( const UMLRTMessage & message );
    void initialize( const UMLRTMessage & message );
    const char * getCurrentStateString() const;

protected:
    enum State
    {
        Connecting,
        Error,
        Waiting,
        SPECIAL_INTERNAL_STATE_TOP,
        SPECIAL_INTERNAL_STATE_UNVISITED
    };
    const char * stateNames[5];
    State currentState;
    UMLRTLogProtocol_baserole log;
	UMLRTMQTTProtocol_baserole mqtt;

    void update_state( State newState );
    void entryaction_____Connecting( const UMLRTMessage * msg );
    void entryaction_____Waiting( const UMLRTMessage * msg );
    void transitionaction_____conn_error( const UMLRTMessage * msg );
    void transitionaction_____connected( const UMLRTMessage * msg );
    void transitionaction_____msg_recv( const UMLRTMessage * msg );
    void transitionaction_____recv_error( const UMLRTMessage * msg );
    void actionchain_____Initial( const UMLRTMessage * msg );
    void actionchain_____conn_error( const UMLRTMessage * msg );
    void actionchain_____connected( const UMLRTMessage * msg );
    void actionchain_____msg_recv( const UMLRTMessage * msg );
    void actionchain_____recv_error( const UMLRTMessage * msg );
    State state_____Connecting( const UMLRTMessage * msg );
    State state_____Error( const UMLRTMessage * msg );
    State state_____Waiting( const UMLRTMessage * msg );
	void proxyMessage ( const UMLRTMessage * msg );
public:
    enum InternalPortId
    {
        internalport_mqtt,
        internalport_log
    };
    enum PartId
    {
    };
    virtual void bindPort( bool isBorder, int portId, int index );
    virtual void unbindPort( bool isBorder, int portId, int index );
    char * host;
    int port;
    char * username;
    char * password;
    char * topic;
    char * subscriptions;
};
extern const UMLRTCapsuleClass MQTTProxyCapsule;

#endif

