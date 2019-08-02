/**
 * Send a request for the websocket port
 * Open a websocket to that port
 */
function startWebsocket() {
  var xmlHTTP                = new XMLHttpRequest();
  xmlHTTP.onreadystatechange = function() {
    if (this.readyState == 4) {
      if (this.status == 200)
      document.getElementById("websocket-port").innerHTML = "Found GUI port at " + this.responseText;
      else
        document.getElementById("websocket-port").innerHTML = "Cannot find GUI port ERROR: " + this.status;
    }
  };
  xmlHTTP.open("GET", "~/websocket", true);
  xmlHTTP.send();
}

window.onload = startWebsocket;