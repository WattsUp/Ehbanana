/**
 * On receiving a message from the WebSocket
 * @param {WebSocket event} event
 */
function webSocketMessage(event) {
  var jsonEvent = JSON.parse(event.data);
  if (jsonEvent.href != window.location.pathname && jsonEvent.href != "")
    return;
  for (var id in jsonEvent.elements) {
    var obj = document.getElementById(id);
    if (!obj) {
      console.log("Ehbanana Message #" + id + " not found");
      continue;
    }
    for (var key in jsonEvent.elements[id]) {
      obj[key] = jsonEvent.elements[id][key];
    }
    if (obj.hasAttribute("ebcallback")) {
      var func = window[obj.getAttribute("ebcallback")];
      if (typeof func == "function")
        func(obj);
    }
  }
  if (document.body.hasAttribute("ebmsg-callback")) {
    var func = window[document.body.getAttribute("ebmsg-callback")];
    if (typeof func == "function")
      func();
  }
}

/**
 * Send a request for the websocket port
 * Open a websocket to that port
 */
function startWebsocket() {
  webSocket           = new WebSocket(webSocketAddress);
  webSocket.onmessage = webSocketMessage;
  webSocket.onclose   = function(event) {
    var webSocketStatus = document.getElementById("websocket-status");
    if (webSocketStatus)
      webSocketStatus.innerHTML = "Closed connection to " + webSocketAddress;
    if (document.body) {
      document.body.classList.remove("ehbanana-error");
      document.body.classList.remove("ehbanana-opened");
      document.body.classList.add("ehbanana-closed");
    }
  };
  webSocket.onopen = function(event) {
    var webSocketStatus = document.getElementById("websocket-status");
    if (webSocketStatus)
      webSocketStatus.innerHTML = "Opened connection to " + webSocketAddress;
    if (document.body) {
      document.body.classList.remove("ehbanana-error");
      document.body.classList.add("ehbanana-opened");
      document.body.classList.remove("ehbanana-closed");
    }
    if (document.readyState == "loading") {
      document.addEventListener("load", attachListeners);
    } else {
      attachListeners();
    }
  };
  webSocket.onerror = function(event) {
    var webSocketStatus = document.getElementById("websocket-status");
    if (webSocketStatus)
      webSocketStatus.innerHTML = "Could not connect to " + webSocketAddress;
    if (document.body) {
      document.body.classList.add("ehbanana-error");
      document.body.classList.remove("ehbanana-opened");
      document.body.classList.remove("ehbanana-closed");
    }
  };
}

/**
 * Listen to an input event and send the results to the WebSocket
 * @param {InputEvent} event
 */
function listenerInput(event) {
  var jsonEvent = {
    href: window.location.pathname,
    id: event.target.id,
    value: "" + event.target.value
  };

  // populate data with appropriate information
  if (event.target.type == "checkbox")
    jsonEvent.value = "" + event.target.checked;

  if (event.target.type == "file") {
    jsonEvent.fileSize = event.target.files[0].size;
  }

  webSocket.send(JSON.stringify(jsonEvent));

  if (event.target.type == "file") {
    webSocket.send(event.target.files[0]); // Sent as binary
  }
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
      elementsInput[i].setAttribute("id", elementsInput[i].name);
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
    var jsonEvent = {
      href: window.location.pathname,
      id: elementsOnload[i].id,
      value: "on-load-update"
    };
    webSocket.send(JSON.stringify(jsonEvent));
  }
}

var webSocket = null;
var webSocketAddress =
    "ws://" + window.location.hostname + ":" + window.location.port;

startWebsocket();