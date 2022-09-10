
#ifndef __HTTML___
#define __HTTML___

#include "pgmspace.h"

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<!DOCTYPE html>

<html>
  <head>
    <style>
      textarea {
        overflow-y: scroll;
        height: 500px;
        width: 500px;
        resize: none; /* Remove this if you want the user to resize the textarea */
      }
    </style>

    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.6.0/jquery.min.js"></script>

    <script type="text/javascript">
      function WebSocketTest() {
        if ("WebSocket" in window) {
          //  alert("WebSocket is supported by your Browser!");

          // Let us open a web socket
          var ws = new WebSocket("ws://192.168.1.30/ws");

          ws.onopen = function () {};

          ws.onmessage = function (evt) {
            var received_msg = evt.data;

            // Check the data source

            if (received_msg.startsWith("RX1,")) {
              $("#textarea1").val(
                $("#textarea1").val() + received_msg.substring(4)
              );
            } else if (received_msg.startsWith("RX2,")) {
              $("#textarea2").val(
                $("#textarea2").val() + received_msg.substring(4)
              );
            } else {
              console.log("Invalid Message");
            }
          };

          ws.onclose = function () {
            // websocket is closed.
            alert("Connection is closed...");
          };
        } else {
          // The browser doesn't support WebSocket
          alert("WebSocket NOT supported by your Browser!");
        }
      }
    </script>
  </head>

  <body>
    <div id="sse">
      <a href="javascript:WebSocketTest()">Start</a>
    </div>

    <div class="row">
      <form class="col s12">
        <div class="row">
          <div class="input-field col s12"></div>
          <label for="textarea1">Console</label>
          <br />
          <textarea id="textarea1"></textarea>
          <textarea id="textarea2"></textarea>

          <br />
          <input
            type="button"
            id="btnClear"
            onClick='document.getElementById("textarea1").value =""; document.getElementById("textarea2").value =""'
            value="Clear"
          />
        </div>
      </form>
    </div>
  </body>
</html>

    )rawliteral";

#endif