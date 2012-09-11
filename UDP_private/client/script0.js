
      helloWorldModule = null;  // Global application object.
      statusText = 'NO-STATUS';

      // Indicate success when the NaCl module has loaded.
      function moduleDidLoad()
      {
          helloWorldModule = document.getElementById('hello_world');
          console.log('SUCCESS');
      }

      // Handle a message coming from the NaCl module.
      function handleMessage(message_event)
      {
          console.log(message_event.data);
      }

     
	  
	   var listener = document.getElementById('listener')
        listener.addEventListener('load', moduleDidLoad, true);
        listener.addEventListener('message', handleMessage, true);
		 document.getElementById('listener')