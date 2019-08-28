/**
 * On receiving a message from the WebSocket
 * @param {WebSocket event} event
 */
function webSocketMessage(event) {
  jsonEvent = JSON.parse(event.data);
  obj       = document.getElementById(jsonEvent.id);
  switch (obj.nodeName) {
    case "DIV":
    case "SPAN":
      obj.innerHTML = jsonEvent.value;
      break;
    case "INPUT":
      obj.value = jsonEvent.value;
      break;
    case "TIME":
      var date = new Date(parseInt(jsonEvent.value));
      console.log(jsonEvent.value);
      obj.innerHTML = date.getHours() + ":" +
                      ("0" + date.getMinutes()).slice(-2) + ":" +
                      ("0" + date.getSeconds()).slice(-2);
    default:
      console.log(obj.nodeName);
  }
  if (obj.hasAttribute("ebcallback")) {
    var func = window[obj.getAttribute("ebcallback")];
    if (typeof func == "function")
      func(jsonEvent.value);
  }
}

/**
 * Send a request for the websocket port
 * Open a websocket to that port
 */
function startWebsocket() {
  webSocketStatus = document.getElementById("websocket-status");
  bodyElement     = document.getElementsByTagName("BODY")[0];
  bodyElement.classList.add("ehbanana-closed");

  webSocket           = new WebSocket(webSocketAddress);
  webSocket.onmessage = webSocketMessage;
  webSocket.onclose   = function(event) {
    webSocketStatus.innerHTML = "Closed connection to " + webSocketAddress;
    bodyElement.classList.remove("ehbanana-error");
    bodyElement.classList.remove("ehbanana-opened");
    bodyElement.classList.add("ehbanana-closed");
  };
  webSocket.onopen = function(event) {
    webSocketStatus.innerHTML = "Opened connection to " + webSocketAddress;
    bodyElement.classList.remove("ehbanana-error");
    bodyElement.classList.add("ehbanana-opened");
    bodyElement.classList.remove("ehbanana-closed");
    attachListeners();
  };
  webSocket.onerror = function(event) {
    webSocketStatus.innerHTML = "Could not connect to " + webSocketAddress;
    bodyElement.classList.add("ehbanana-error");
    bodyElement.classList.remove("ehbanana-opened");
    bodyElement.classList.remove("ehbanana-closed");
  };
}

/**
 * Listen to an input event and send the results to the WebSocket
 * @param {InputEvent} event
 */
function listenerInput(event) {
  var jsonEvent = {id: event.target.id, value: event.target.value};
  if (event.target.type == "checkbox")
    jsonEvent.checked = event.target.checked;
  else if (event.target.type == "file") {
    jsonEvent.fileSize = event.target.files[0].size;
    webSocket.send(JSON.stringify(jsonEvent));
    webSocket.send(event.target.files[0]); // Sent as binary
    return;
  }
  webSocket.send(JSON.stringify(jsonEvent));
}

/**
 * Add a listener to each element with "eb-*" class
 */
function attachListeners() {
  var elementsInput = document.getElementsByClassName("eb-input");
  for (var i = 0; i < elementsInput.length; i++) {
    if (elementsInput[i].getAttribute("id") == null) {
      if (elementsInput[i].getAttribute("name") == null) {
        console.log("No name nor id for eb-input:", elementsInput[i]);
        continue;
      }
      elementsInput[i].setAttribute("is", elementsInput[i].name);
    }
    if (elementsInput[i].getAttribute("type") == "button")
      elementsInput[i].addEventListener("click", listenerInput);
    else
      elementsInput[i].addEventListener("input", listenerInput);
  }
  var elementsOnload = document.getElementsByClassName("eb-onload");
  for (var i = 0; i < elementsOnload.length; i++) {
    if (elementsOnload[i].getAttribute("id") == null) {
      if (elementsOnload[i].getAttribute("name") == null) {
        console.log("No name nor id for eb-onload:", elementsOnload[i]);
        continue;
      }
      elementsOnload[i].setAttribute("id", elementsOnload[i].name);
    }
    var jsonEvent = {id: elementsOnload[i].id, value: "on-load-update"};
    webSocket.send(JSON.stringify(jsonEvent));
  }
}

window.addEventListener("load", startWebsocket);

var webSocket = null;
var webSocketAddress =
    "ws://" + window.location.hostname + ":" + window.location.port;
var webSocketStatus = null;
var bodyElement     = null;