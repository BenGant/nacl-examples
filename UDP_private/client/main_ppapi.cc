


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
#include "ppapi/c/pp_resource.h"
#include "ppapi/c/ppb.h"
#include "ppapi/c/ppb_instance.h"
#include "ppapi/c/ppb_messaging.h"
#include "ppapi/c/ppb_var.h"
#include "ppapi/c/ppp.h"
#include "ppapi/c/ppb_core.h"

#include "ppapi/c/ppp_instance.h"
#include "ppapi/c/ppp_messaging.h"
#include "ppapi/c/pp_input_event.h"

#include "ppapi/c/pp_completion_callback.h"

//UDP specific - the 'private' folder was copied from the chromium src/ppapi/c headers folder
#include "ppapi/c/private/ppb_udp_socket_private.h"

static PPB_Messaging* ppb_messaging_interface = NULL;
static PPB_Var* ppb_var_interface = NULL;
static PPB_Core* ppb_core_interface = NULL;
static PPB_Instance* ppb_instance_interface = NULL;
static PPP_Messaging* ppp_messaging_interface = NULL;
static PPB_UDPSocket_Private* ppb_udpsocket_interface = NULL;
static PPB_NetAddress_Private* ppb_netaddress_interface = NULL;

int gServerPort = 32000;
int gClientPort = 32001;

PP_Instance appInstance_;	
PP_Resource sockInstance_;
PP_Module moduleInstance_;
PP_NetAddress_Private client_addr;
PP_NetAddress_Private server_addr;

//-----------------------------------------------------------------------------

void init(void);
void shutDown(void);
void onRecvFrom(void* pData, int32_t dataSize);

#define BUFFER_SIZE 1000
char recv_buffer[BUFFER_SIZE];
char send_buffer[BUFFER_SIZE];
PP_CompletionCallback recvcc = PP_MakeCompletionCallback(onRecvFrom, 0);
//-----------------------------------------------------------------------------
void onRecvFrom(void* pData, int32_t dataSize)
{
	//any error codes will be given to us in the dataSize value; see pp_errors.h for a list of errors
	if(dataSize <=0 || !pData )
	{
		if(dataSize==-2)
			fprintf(stdout,"onRecvFrom datasize == -2;\n");
		fprintf(stderr,"onRecvFrom ; reissuing\n");

		
		ppb_udpsocket_interface->RecvFrom(sockInstance_, &recv_buffer[0], BUFFER_SIZE, recvcc);

		return;
	}


	
	fprintf(stdout,"onRecvFrom %i:%s\n", dataSize, pData );
}
//-----------------------------------------------------------------------------
void onSendTo(void* pData, int32_t dataSize)
{
	//any error codes will be given to us in the dataSize value; see pp_errors.h for a list of errors
	if(dataSize ==-2)
	{
		fprintf(stderr,"onSendTo exiting, datasize == -2\n");
		return;
	}

	//this will wait for the server to send us something back
	ppb_udpsocket_interface->RecvFrom(sockInstance_, &recv_buffer[0], BUFFER_SIZE, recvcc);

}

//-----------------------------------------------------------------------------
void onSocketBound(void* pData, int32_t dataSize)
{
	//any error codes will be given to us in the dataSize value
	if(dataSize ==-2)
	{
		fprintf(stderr,"onSocketBound exiting, datasize == -2\n");
		return;
	}
	
	//send data to the server
	PP_CompletionCallback cc = PP_MakeCompletionCallback(onSendTo, 0);
	const int32_t res = ppb_udpsocket_interface->SendTo(sockInstance_,&send_buffer[0],(int)BUFFER_SIZE,&server_addr,cc);
	 
}


//-----------------------------------------------------------------------------
void init( void )
{

	//create the socket
	sockInstance_ = ppb_udpsocket_interface->Create(appInstance_);

	//create an adress for the server, where we're going to connect to
	uint8_t ip[4] = {127,0,0,1};
	ppb_netaddress_interface->CreateFromIPv4Address(reinterpret_cast<const uint8_t*>(&ip[0]), gServerPort, &server_addr);

	//create an address for us (the client) this can be any address we have
	ppb_netaddress_interface->GetAnyAddress(PP_FALSE,&client_addr);
	ppb_netaddress_interface->ReplacePort(&client_addr,gClientPort,&client_addr);

	//create a message
	char* msg = "THIS IS A MESSAGE FROM THE FUTURE\n\0";
	strcpy(&send_buffer[0],msg);
	

	//bind a socket to the client address
	PP_CompletionCallback cc = PP_MakeCompletionCallback(onSocketBound, 0);
	const int32_t res = ppb_udpsocket_interface->Bind(sockInstance_,&client_addr, cc);	//THIS RETURNS -1.. which is PP_OK_COMPLETIONPENDING
	fprintf(stdout,"bind = %i\n",res);




}
 


//-----------------------------------------------------------------------------
void shutDown( void )
{
	ppb_udpsocket_interface->Close(sockInstance_);
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

 init();

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
}

//-----------------------------------------------------------------------------
PP_EXPORT int32_t PPP_InitializeModule(PP_Module a_module_id, PPB_GetInterface get_browser) 
{
	moduleInstance_ = a_module_id;
  ppb_messaging_interface = (PPB_Messaging*)(get_browser(PPB_MESSAGING_INTERFACE));
  ppb_var_interface = (PPB_Var*)(get_browser(PPB_VAR_INTERFACE));
  ppb_instance_interface = (PPB_Instance*)get_browser(PPB_INSTANCE_INTERFACE);
  ppb_core_interface = (PPB_Core*)get_browser(PPB_CORE_INTERFACE);
  ppp_messaging_interface = (PPP_Messaging*)(get_browser(PPP_MESSAGING_INTERFACE)); 

  //make sure we create these two other interfaces
  ppb_udpsocket_interface = (PPB_UDPSocket_Private*)(get_browser(PPB_UDPSOCKET_PRIVATE_INTERFACE_0_3)); 
  ppb_netaddress_interface = (PPB_NetAddress_Private*)(get_browser(PPB_NETADDRESS_PRIVATE_INTERFACE_1_1)); 

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
