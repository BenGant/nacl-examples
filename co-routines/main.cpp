#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>



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
#include <memory.h>

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
#include "ppapi/utility/completion_callback_factory.h"

static PPB_Messaging* ppb_messaging_interface = NULL;
static PPB_Var* ppb_var_interface = NULL;
static PPB_Core* ppb_core_interface = NULL;
static PPB_Instance* ppb_instance_interface = NULL;
static PPP_Messaging* ppp_messaging_interface = NULL;
int g_ResourceLoadedCounter = 0;

PP_Instance appInstance_;	



//coroutine namespace which will manage timeslicing the execution
namespace coroutine
{
	enum {
		kStackSize = 1<<16
	};

	static jmp_buf mainLabel, childLabel;
	//-----------------------------------
	void BlockPseudoThread(void) {
		if (!setjmp(childLabel) )
		{	
			longjmp(mainLabel,1);
		} 
	}
	//-----------------------------------
	void ResumePseudoThread(void) {
		if (!setjmp(mainLabel) )
		{	
			longjmp(childLabel,1);
		} 
	}
	//-----------------------------------
	void CreatePseudoThread(void (*start)(void)) {
		if (!setjmp(mainLabel) )
		{	
   		alloca(kStackSize);
			start();
			longjmp(mainLabel,1);
		} 
	}
	//-----------------------------------
	void update(void* foo, int bar) 
	{
		ResumePseudoThread();
	}
	//-----------------------------------
	PP_CompletionCallback updateCallback = PP_MakeCompletionCallback(update, 0);
	//-----------------------------------------------------------------------------
	void Flush(void) {
		  ppb_core_interface->CallOnMainThread(0, updateCallback, 0);
		  BlockPseudoThread();
	}
	
}

//-----------------------------------------------------------------------------
void onFileLoaded(void* pData, int32_t dataSize)
{
	// process data returned from file loading

	//once all the data has been read, we can regain execution control
	coroutine::ResumePseudoThread();
}

//-----------------------------------------------------------------------------
void blockingURLRead()
{

   //rather than actually calling readFileFromURL, we simulate some arbitrary Pepper API function by just calling 'callonmainthread'
	PP_CompletionCallback fileLoadedCB = PP_MakeCompletionCallback(onFileLoaded, 0);
	ppb_core_interface->CallOnMainThread(0, fileLoadedCB, 0);

	//now that the async call has been kicked off to chrome, we yield execution control back to chrome so it can service it
	coroutine::BlockPseudoThread();

	printf("I AM A CHEEZEBURGER");
}

//-----------------------------------------------------------------------------
void userUpdate(void) {
  for (;;) {
	  
	  //..some code goes here....
	
	  //randomly kick off a file load call for whatever reason
		if(rand() %50 == 25)
			blockingURLRead();

		//some other code goes here....


		//IMPORTANT, at the end of the while-loop, we need to signal execution control back to chrome for processing	
	  coroutine::Flush();
  }
}
//-----------------------------------------------------------------------------
void init( void )
{
	//called when the app boots. Allows user to own their control loop
	coroutine::CreatePseudoThread(userUpdate);
}

//-----------------------------------------------------------------------------

static PP_Bool Instance_DidCreate(PP_Instance instance,
                                  uint32_t argc,
                                  const char* argn[],
                                  const char* argv[]) {
	appInstance_ = instance;
	init();
  return PP_TRUE;
}
//-----------------------------------------------------------------------------
static void Instance_DidDestroy(PP_Instance instance) {}
//-----------------------------------------------------------------------------
static void Instance_DidChangeView(PP_Instance instance, PP_Resource view_resource) {}
//-----------------------------------------------------------------------------
static void Instance_DidChangeFocus(PP_Instance instance, PP_Bool has_focus) {}
//-----------------------------------------------------------------------------
static PP_Bool Instance_HandleDocumentLoad(PP_Instance instance, PP_Resource url_loader) {  return PP_FALSE;}
//-----------------------------------------------------------------------------
PP_Bool InputEvent_HandleEvent(PP_Instance instance_id, PP_Resource input_event) {  return PP_TRUE;}
//-----------------------------------------------------------------------------
static void Messaging_HandleMessage(PP_Instance instance, struct PP_Var message) {}
//-----------------------------------------------------------------------------
PP_EXPORT int32_t PPP_InitializeModule(PP_Module a_module_id, PPB_GetInterface get_browser) 
{
  ppb_messaging_interface = (PPB_Messaging*)(get_browser(PPB_MESSAGING_INTERFACE));
  ppb_var_interface = (PPB_Var*)(get_browser(PPB_VAR_INTERFACE));
  ppb_instance_interface = (PPB_Instance*)get_browser(PPB_INSTANCE_INTERFACE);
  ppb_core_interface = (PPB_Core*)get_browser(PPB_CORE_INTERFACE);
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
PP_EXPORT void PPP_ShutdownModule() {}


