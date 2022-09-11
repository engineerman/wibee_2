
#ifndef __HTTML___
#define __HTTML___

#include "pgmspace.h"

const char index_html[] PROGMEM = R"rawliteral(
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

      .column {
        float: left;
        width: 50%;
      }

      /* Clear floats after the columns */
      .row:after {
        content: "";
        display: table;
        clear: both;
      }
    </style>

    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.6.0/jquery.min.js"></script>

    <script type="text/javascript">
      console.log(window.location.hostname);

      var ws;

      function sendMessage() {
        var cmd = document.getElementById("txtCmd").value;

        ws.send(cmd + "\n");
      }

      function WebSocketTest() {
        if ("WebSocket" in window) {
          //  alert("WebSocket is supported by your Browser!");

          // Let us open a web socket
          if (window.location.hostname.startsWith("127.0.0.1")) {
            ws = new WebSocket("ws://192.168.1.30/ws");
          } else {
            ws = new WebSocket("ws://" + window.location.hostname + "/ws");
          }

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
      <div class="column">
        <label for="textarea1">UART1</label>
        <br />
        <textarea id="textarea1"></textarea>
      </div>
      <div class="column">
        <label for="textarea1">UART2</label>
        <br />
        <textarea id="textarea2"></textarea>
      </div>
    </div>

    <input
      type="button"
      id="btnClear"
      onClick='document.getElementById("textarea1").value =""; document.getElementById("textarea2").value =""'
      value="Clear"
    />

    <br />
    <br />

    <input placeholder="Command" id="txtCmd" type="text" class="validate" />

    <input type="button" id="btnSned" onClick="sendMessage()" value="Send" />
  </body>
</html>


    )rawliteral";

#endif