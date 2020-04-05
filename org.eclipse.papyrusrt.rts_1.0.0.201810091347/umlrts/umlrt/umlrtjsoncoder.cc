// umlrtjsoncoder.cc

#include "basedebug.hh"
#include "basefatal.hh"
#include "umlrtjsoncoder.hh"
#include "umlrtsignalregistry.hh"

bool UMLRTJSONCoder::fromJSON( const char * json, const UMLRTCommsPort *& port, UMLRTSignal & signal )
{
	Document document;
	document.Parse(json);

	if(!document.IsObject()) {
		fprintf(stderr, "Error parsing JSON message: '%s'\n", json);
		return false;
	}

	if(!document.HasMember("signal")) {
		fprintf(stderr, "Error parsing JSON message: '%s'\n", json);
		return false;
	}

	const Value& sigVal = document["signal"];
	if(!sigVal.IsString()) {
		fprintf(stderr, "Erroneous JSON message: '%s'\n", json);
		return false;
	}

	const Value& protVal = document["protocol"];
	if(!protVal.IsString()) {
		fprintf(stderr, "Erroneous JSON message: '%s'\n", json);
		return false;
	}

	const char* signalName = sigVal.GetString();
	const char* protocolName = protVal.GetString();

	int signalID = UMLRTSignalRegistry::getRegistry().getInSignalID(protocolName, signalName);
	if(signalID == -1)
		return false;

	UMLRTObject * payloadObj = UMLRTSignalRegistry::getRegistry().getInSignalPayloadObject(protocolName, signalName);
	if(payloadObj->sizeOf == 0)
		payloadObj = NULL;

	if(payloadObj != NULL && !document.HasMember("params")) {
		fprintf(stderr, "Mismatched signal parameters in JSON message '%s'\n", json);
		return false;
	}

	if(payloadObj == NULL && document.HasMember("params")) {
		const Value& paramsArr = document["params"];
		if(!paramsArr.IsArray()) {
			fprintf(stderr, "Erroneous JSON message: '%s'\n", json);
			return false;
		}

		if(paramsArr.Size() > 0) {
			fprintf(stderr, "Mismatched signal parameters in JSON message '%s'\n", json);
			return false;
		}
	}

	if(payloadObj != NULL) {
		const Value& paramsArr = document["params"];
		if(!paramsArr.IsArray()) {
			fprintf(stderr, "Erroneous JSON message: '%s'\n", json);
			return false;
		}

		if(payloadObj->numFields != paramsArr.Size()) {
			fprintf(stderr, "Mismatched signal parameters in JSON message '%s'\n", json);
			return false;
		}

		uint8_t* data = (uint8_t*) malloc(sizeof(uint8_t) * payloadObj->sizeOf);
		if(data == NULL)
			FATAL("memory allocation failed for payload data");

		for (size_t i=0; i<payloadObj->numFields; i++)
	    {
	    	const Value& paramObj = paramsArr[i];
	    	if(!paramObj.IsObject()) {
	    		fprintf(stderr, "Erroneous JSON message: '%s'\n", json);
	    		return false;
	    	}

	    	if(!paramObj.HasMember("value")) {

	    		fprintf(stderr, "Erroneous JSON message: '%s'\n", json);
	    		return false;
	    	}

	    	if(paramObj.HasMember("type")
	    		&& !paramObj["type"].IsString()) {

	    		fprintf(stderr, "Erroneous JSON message: '%s'\n", json);
	    		return false;
	    	}

	    	// decode
	    	if(!decodeParam(paramObj, payloadObj->fields[i].desc, data + payloadObj->fields[i].offset)) {
	    		fprintf(stderr, "Mismatched signal parameters in JSON message '%s'\n", json);
	    		return false;
	    	}
	    }

		signal.initialize(signalName, signalID, port, payloadObj, data);
		free(data);
	}

	else {
		signal.initialize(signalName, signalID, port);
	}

	return true;
}

bool UMLRTJSONCoder::decodeParam( const Value & param, const UMLRTObject_class * desc, uint8_t * buffer) {
	const Value& paramVal = param["value"];

	if(param.HasMember("type")
		&& strcmp(param["type"].GetString(), desc->name) != 0)
		return false;

	if(paramVal.IsArray()) {
		if(paramVal.Size() != desc->object.numFields)
			return false;

		for (size_t i=0; i<desc->object.numFields; i++) {
	    	const Value& paramObj = paramVal[i];

	    	if(!paramObj.HasMember("value"))
	    		return false;

	    	if(paramObj.HasMember("type")
	    		&& !paramObj["type"].IsString())
	    		return false;

	    	if(!decodeParam(paramObj, desc->object.fields[i].desc, buffer + desc->object.fields[i].offset))
	    		return false;
	    }

	}

	else if(strcmp(desc->name, "bool") == 0) {
		if(!paramVal.IsBool())
			return false;

		bool b = paramVal.GetBool();
		desc->copy(desc, &b, buffer);
	}

	else if(strcmp(desc->name, "char") == 0) {
		if(!paramVal.IsString())
			return false;

		const char* str = paramVal.GetString();
		if(strlen(str) != 1)
			return false;

		char c = str[0];
		desc->copy(desc, &c, buffer);
	}

	else if(strcmp(desc->name, "double") == 0) {
		if(!paramVal.IsDouble())
			return false;

		double d = paramVal.GetDouble();
		desc->copy(desc, &d, buffer);
	}

	else if(strcmp(desc->name, "float") == 0) {
		if(!paramVal.IsFloat())
			return false;

		float f = paramVal.GetFloat();
		desc->copy(desc, &f, buffer);
	}

	else if(strcmp(desc->name, "int") == 0) {
		if(!paramVal.IsInt())
			return false;

		int i = paramVal.GetInt();
		desc->copy(desc, &i, buffer);
	}

	else if(strcmp(desc->name, "charptr") == 0) {
		if(!paramVal.IsString())
			return false;

		const char* str = paramVal.GetString();
		desc->copy(desc, &str, buffer);
	}

	else
		return false;

	return true;
}

char * UMLRTJSONCoder::toJSON( const UMLRTSignal & signal ) {
	Document document;
	document.SetObject();
	Document::AllocatorType& allocator = document.GetAllocator();

	Value paramsArray(kArrayType);

	const UMLRTObject * payloadObj = UMLRTSignalRegistry::getRegistry().getOutSignalPayloadObject(
		signal.getSrcPort()->role()->protocol, signal.getName());

	if(payloadObj == NULL)
		FATAL("cannot encode signal with invalid payload object");

	for(size_t i=0; i<payloadObj->numFields; i++) {
		Value paramObj(kObjectType);

		Value paramName;
		paramName.SetString(payloadObj->fields[i].name, allocator);
		paramObj.AddMember("name", paramName, allocator);

		encodeParam(document, paramObj, signal.getType(i), signal.getParam(i));
		paramsArray.PushBack(paramObj, allocator);
	}

	Value signalName;
	signalName.SetString(signal.getName(), allocator);

	Value protocolName;
	protocolName.SetString(signal.getSrcPort()->role()->protocol, allocator);

	document.AddMember("protocol", protocolName, allocator);
	document.AddMember("signal", signalName, allocator);
	document.AddMember("params", paramsArray, allocator);

	StringBuffer strbuf;
	Writer<StringBuffer> writer(strbuf);
	document.Accept(writer);

	return strdup(strbuf.GetString());
}

void UMLRTJSONCoder::encodeParam( Document & document, Value & paramObj, const UMLRTObject_class * desc, void * data ) {
	Document::AllocatorType& allocator = document.GetAllocator();

	Value paramType;
	paramType.SetString(desc->name, allocator);
	paramObj.AddMember("type", paramType, allocator);

	if (desc->object.numFields != 0) {	// complex object
		Value valueArray(kArrayType);

    	for (size_t i=0; i<desc->object.numFields; i++) {
            const UMLRTObject_field* field = &desc->object.fields[i];

            Value subParamObj(kObjectType);

            Value paramName;
			paramName.SetString(field->name, allocator);
			subParamObj.AddMember("name", paramName, allocator);

            encodeParam(document, subParamObj, field->desc, (uint8_t*) data + field->offset);
			valueArray.PushBack(subParamObj, allocator);
        }

       	paramObj.AddMember("value", valueArray, allocator);
    }

	else if(strcmp(desc->name, "bool") == 0) {
	    bool b;
	    desc->copy(desc, data, &b);

	    Value paramValue;
		paramValue.SetBool(b);
		paramObj.AddMember("value", paramValue, allocator);
	}

	else if(strcmp(desc->name, "char") == 0) {
	    Value paramValue;
		paramValue.SetString((char*) data, 1, allocator);
		paramObj.AddMember("value", paramValue, allocator);
	}

	else if(strcmp(desc->name, "double") == 0) {
	    double d;
        desc->copy(desc, data, &d);


	    Value paramValue;
		paramValue.SetDouble(d);
		paramObj.AddMember("value", paramValue, allocator);
	}

	else if(strcmp(desc->name, "float") == 0) {
		float f;
        desc->copy(desc, data, &f);


	    Value paramValue;
		paramValue.SetFloat(f);
		paramObj.AddMember("value", paramValue, allocator);
	}

	else if(strcmp(desc->name, "int") == 0) {
		int iv;
        desc->copy(desc, data, &iv);


	    Value paramValue;
		paramValue.SetInt(iv);
		paramObj.AddMember("value", paramValue, allocator);
	}

	else if(strcmp(desc->name, "charptr") == 0) {
		void* ptr;
		desc->copy(desc, data, &ptr);

		Value paramValue;
		paramValue.SetString((const char*) ptr, allocator);
		paramObj.AddMember("value", paramValue, allocator);

		desc->destroy(desc, &ptr);
	}

	else {
		fprintf(stderr, "Unsupported data type '%s'\n", desc->name);
	}
}
