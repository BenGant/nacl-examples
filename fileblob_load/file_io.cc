


#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>


#include <cstdio>
#include <sstream>
#include <iostream>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>

#include "ppapi/c/pp_errors.h"
#include "ppapi/c/pp_module.h"
#include "ppapi/c/pp_var.h"
#include "ppapi/c/ppb.h"
#include "ppapi/c/ppb_instance.h"
#include "ppapi/c/ppb_messaging.h"
#include "ppapi/c/ppb_var.h"
#include "ppapi/c/ppb_var_array_buffer.h"
#include "ppapi/c/ppp.h"
#include "ppapi/c/ppb_core.h"
#include "ppapi/c/ppp_instance.h"
#include "ppapi/c/ppp_messaging.h"
#include "ppapi/c/pp_input_event.h"

#include "ppapi/c/pp_completion_callback.h"
#include "ppapi/c/ppb_url_loader.h"
#include "ppapi/c/ppb_url_request_info.h"

//int main(){return 0;}

//#include "ppapi/utility/completion_callback_factory.h"
//pp::Module::Get()->core()->CallOnMainThread(0, pp::CompletionCallback(initGLES_mt, (void*)attribs,0), PP_OK);

static PPB_Messaging* ppb_messaging_interface = NULL;
static PPB_Var* ppb_var_interface = NULL;
static PPB_VarArrayBuffer* ppb_varArrayBuffer_interface = NULL;
static PPB_Core* ppb_core_interface = NULL;
static PPB_Instance* ppb_instance_interface = NULL;
static PPB_URLRequestInfo* ppb_urlreq_interface = NULL;
static PPB_URLLoader* ppb_urlloader_interface = NULL;
static PPP_Messaging* ppp_messaging_interface = NULL;
int g_ResourceLoadedCounter = 0;

PP_Instance appInstance_;	


//-----------------------------------------------------------------------------
static const char* VarToCStr(const PP_Var var) {
  if (ppb_var_interface == NULL) 
	  return 0;

	  uint32_t len;
	 const char* blobURL = ppb_var_interface->VarToUtf8 ( var, &len);
    return blobURL;
}
//-----------------------------------------------------------------------------
static struct PP_Var CStrToVar(const char* str) {
  if (ppb_var_interface != NULL) {
    return ppb_var_interface->VarFromUtf8(str, strlen(str));
  }
  return PP_MakeUndefined();
}

//-----------------------------------------------------------------------------
#define READBUFFERSIZE 1024*50
struct fileReqObj
{
	PP_Resource urlRequestContext_;
	PP_Resource urlLoadContext_;
	void (*pCallback_)(void* pData, int32_t dataSize);
	char readBuffer_[READBUFFERSIZE];
	unsigned int dataBufferSize_;
};
void readFileFromURL(const char* pURL, void (pCallback)(void* pData, int32_t dataSize));



//-----------------------------------------------------------------------------

void init(void);
void shutDown(void);


//-----------------------------------------------------------------------------
void onFileLoaded(void* pData, int32_t dataSize)
{
	if(dataSize <=0)
		return;

	//CLM HACK - one we have the data, simply pass it back to javascript.
	PP_Var v2 = ppb_varArrayBuffer_interface->Create(dataSize);

	uint32_t byte_length;
	ppb_varArrayBuffer_interface->ByteLength(v2, &byte_length);

	void* pDst = ppb_varArrayBuffer_interface->Map(v2);
	memcpy(pDst,pData,dataSize);
	ppb_varArrayBuffer_interface->Unmap(v2);
	ppb_messaging_interface->PostMessage(appInstance_,v2);
	
}
//-----------------------------------------------------------------------------
void loadBlobFile(const char* pBlobName)
{

	if(pBlobName == 0 || strlen(pBlobName) ==0)
		return;


	//kick off loads from our files
	readFileFromURL(pBlobName,onFileLoaded );

}
//-----------------------------------------------------------------------------
int initInstance()
{
	


	return 0;
}

//-----------------------------------------------------------------------------
void init( void )
{



}



//-----------------------------------------------------------------------------
void shutDown( void )
{

}

	 



//-----------------------------------------------------------------------------

static PP_Bool Instance_DidCreate(PP_Instance instance,
                                  uint32_t argc,
                                  const char* argn[],
                                  const char* argv[]) {

	appInstance_ = instance;

  return PP_TRUE;
}
//-----------------------------------------------------------------------------

static void Instance_DidDestroy(PP_Instance instance) 
{

}
//-----------------------------------------------------------------------------

static void Instance_DidChangeView(PP_Instance instance,
                                   PP_Resource view_resource) 
{

 

}
//-----------------------------------------------------------------------------
static void Instance_DidChangeFocus(PP_Instance instance,
                                    PP_Bool has_focus) {
}

//-----------------------------------------------------------------------------
static PP_Bool Instance_HandleDocumentLoad(PP_Instance instance,
                                           PP_Resource url_loader) {
  /* NaCl modules do not need to handle the document load function. */
  return PP_FALSE;
}
//-----------------------------------------------------------------------------
PP_Bool InputEvent_HandleEvent(PP_Instance instance_id, PP_Resource input_event) {

  return PP_TRUE;
}
//-----------------------------------------------------------------------------
static void Messaging_HandleMessage(PP_Instance instance, struct PP_Var message) 
{
	uint32_t len;
	const char* blobURL = VarToCStr ( message);
	loadBlobFile(blobURL);
}

//-----------------------------------------------------------------------------
PP_EXPORT int32_t PPP_InitializeModule(PP_Module a_module_id, PPB_GetInterface get_browser) 
{
  ppb_messaging_interface = (PPB_Messaging*)(get_browser(PPB_MESSAGING_INTERFACE));
  ppb_var_interface = (PPB_Var*)(get_browser(PPB_VAR_INTERFACE));
  ppb_varArrayBuffer_interface= (PPB_VarArrayBuffer*)(get_browser(PPB_VAR_ARRAY_BUFFER_INTERFACE));
  ppb_instance_interface = (PPB_Instance*)get_browser(PPB_INSTANCE_INTERFACE);
  ppb_core_interface = (PPB_Core*)get_browser(PPB_CORE_INTERFACE);
  ppb_urlreq_interface= (PPB_URLRequestInfo*)(get_browser(PPB_URLREQUESTINFO_INTERFACE)); 
  ppb_urlloader_interface = (PPB_URLLoader*)(get_browser(PPB_URLLOADER_INTERFACE)); 
  ppp_messaging_interface = (PPP_Messaging*)(get_browser(PPP_MESSAGING_INTERFACE)); 
  

  return PP_OK;
}

//-----------------------------------------------------------------------------

PP_EXPORT const void* PPP_GetInterface(const char* interface_name) {
  static PPP_Instance instance_interface = {
    &Instance_DidCreate,
    &Instance_DidDestroy,
    &Instance_DidChangeView,
    &Instance_DidChangeFocus,
    &Instance_HandleDocumentLoad
  };

  if (strcmp(interface_name, PPP_INSTANCE_INTERFACE) == 0)
    return &instance_interface;

   if (strcmp(interface_name, PPP_MESSAGING_INTERFACE) == 0) {
    static PPP_Messaging messaging_interface = {
      &Messaging_HandleMessage,
    };
    return &messaging_interface;
  }
  
  return NULL;
}


//-----------------------------------------------------------------------------
PP_EXPORT void PPP_ShutdownModule() {

}

//-----------------------------------------------------------------------------
void readFileBody(void* pData, int32_t dataSize)
{
	fileReqObj* pFR = (fileReqObj*) pData;

	int64_t bytes_received = 0;
	int64_t total_bytes_to_be_received = 0;
	ppb_urlloader_interface->GetDownloadProgress(pFR->urlLoadContext_,&bytes_received,&total_bytes_to_be_received);

	//if the entire file is read, then we should be G2G
	if(total_bytes_to_be_received == bytes_received)
	{
		pFR->pCallback_(pFR->readBuffer_,total_bytes_to_be_received);
		ppb_urlloader_interface->Close(pFR->urlLoadContext_);
		delete pFR;	//remove from the heap
	}
	else
	{
		//Because the ReadResponseBody function can do a partial read, we force another read call
		const unsigned int bufferReadSize = READBUFFERSIZE;	//set to this for this demo...
		PP_CompletionCallback cc = PP_MakeCompletionCallback(readFileBody, pFR);
		ppb_urlloader_interface->ReadResponseBody(pFR->urlLoadContext_, &pFR->readBuffer_[bytes_received], bufferReadSize - bytes_received, cc);
	}

}

//-----------------------------------------------------------------------------
void readFileResp(void* pData, int32_t dataSize)
{
	fileReqObj* pFR = (fileReqObj*) pData;
	pFR->dataBufferSize_ = 0;

	const unsigned int bufferReadSize = READBUFFERSIZE;	//set to this for this demo...
	PP_CompletionCallback cc = PP_MakeCompletionCallback(readFileBody, pFR);
	ppb_urlloader_interface->ReadResponseBody(pFR->urlLoadContext_, &pFR->readBuffer_[0], bufferReadSize, cc);
}
//-----------------------------------------------------------------------------
void readFileFromURL(const char* pURL, void (pCallback)(void* pData, int32_t dataSize))
{
	fileReqObj* pFR = new fileReqObj;	//create this off the heap

	pFR->pCallback_ = pCallback;

	//	-# Call Create() to create a URLLoader object.
	pFR->urlLoadContext_ = ppb_urlloader_interface->Create(appInstance_);
	
	//We need to set some specific properties on this url request
	pFR->urlRequestContext_ = ppb_urlreq_interface->Create(appInstance_);
	ppb_urlreq_interface->SetProperty(pFR->urlRequestContext_, PP_URLREQUESTPROPERTY_URL, CStrToVar(pURL));
	ppb_urlreq_interface->SetProperty(pFR->urlRequestContext_, PP_URLREQUESTPROPERTY_METHOD, CStrToVar("GET"));	//specifies we're a fetch operation

	PP_Var pv;
	pv.padding=0;
	pv.value.as_bool = PP_TRUE;
	pv.type = PP_VARTYPE_BOOL;
	ppb_urlreq_interface->SetProperty(pFR->urlRequestContext_, PP_URLREQUESTPROPERTY_RECORDDOWNLOADPROGRESS, pv); //specifies we want to get download progress for the file
	
	
	//* -# Call Open() with the <code>URLRequestInfo</code> as an argument.
	PP_CompletionCallback cc = PP_MakeCompletionCallback(readFileResp, pFR);
	ppb_urlloader_interface->Open(pFR->urlLoadContext_, pFR->urlRequestContext_, cc);
}
