const char *indexHtml = R"====(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Stemmett Nixie Device</title>
<style>
  body {
    background-color: #333;
    color: #f5f5f5;
    font-family: Verdana, sans-serif;
    padding: 20px;
  }
  .container {
    max-width: 600px;
    margin: auto;
  }
  .switch-container {
    margin: 20px 0;
  }
  .switch {
    position: relative;
    display: inline-block;
    width: 60px;
    height: 30px;
    margin: 4px 2px;
  }
  .switch input { 
    opacity: 0;
    width: 0;
    height: 0;
  }
  .slider {
    position: absolute;
    cursor: pointer;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background-color: #ccc;
    -webkit-transition: .4s;
    transition: .4s;
  }
  .slider:before {
    position: absolute;
    content: "";
    height: 22px;
    width: 22px;
    left: 4px;
    bottom: 4px;
    background-color: white;
    -webkit-transition: .4s;
    transition: .4s;
  }
  input:checked + .slider {
    background-color: #f90;
  }
  input:focus + .slider {
    box-shadow: 0 0 1px #f90;
  }
  input:checked + .slider:before {
    -webkit-transform: translateX(30px);
    -ms-transform: translateX(30px);
    transform: translateX(30px);
  }
  .slider.rounded {
    border-radius: 8px;
  }
  .slider.rounded:before {
    border-radius: 25%;
  }
  .button {
    background-color: #f90;
    border: none;
    color: white;
    padding: 10px 20px;
    text-align: center;
    text-decoration: none;
    display: inline-block;
    margin: 4px 2px;
    cursor: pointer;
    border-radius: 4px;
  }
  .label {
    vertical-align: middle;
    margin-right: 10px;
  }
</style>
</head>
<body>
<div class="container">
  <h2>Stemmett Nixie Device</h2>

  <button class="button" onclick="restartDevice()">Restart</button>
  <button class="button" onclick="syncTime()">Sync Time</button>

  <div class="switch-container">
    <p><b>Animations:</b></p>
    <div>
      Transition Type:
      <select id="transitionType" onchange="setTransitionType(this.value)">
        <option value="random">Random</option>
        <option value="sequential">Sequential</option>
      </select>
    </div>
  </div>
  <div>
    <p><b>System Time:</b></p>
    <span id="systemTime" class="label"></span>
    <span id="isNight" class="label"></span></br>
    NTP sync: <span id="isNtpSynced" class="label"></span>
  </div>
  <br/>
  <div>
    <p><b>Light Sensor:</b></p>
    Reading: <span id="lightSensorReading" class="label"></span></br>
    Threshold: <input id="lightSensorThreshold" type="number" min="0" max="4096" onchange="setLightSensorThreshold(this.value)">
  </div>
  <br/>
  <div>
    <p><b>Day Mode Settings:</b></p>
    <div>
      Transition Time:
      <input id="dayTransitionTime" type="time" onchange="setTransitionTime('Day', this.value)">
    </div>
    <br/>
    <div>
      Brightness:
      <input id="dayBrightness" type="range" min="0" max="100" value="50" onchange="setBrightness('Day', this.value)">
    </div>
    <br/>
    <div>
      Speed:
      <input id="dayMaxSpeed" type="range" min="0" max="100" value="100" onchange="setMaxSpeed('Day', this.value)">
    </div>
    <br/>
    <div id="animationDaySwitches"></div>
  </div>
  <br/>
  <div>
    <p><b>Night Mode Settings:</b></p>
    <div>
      Transition Time:
      <input id="nightTransitionTime" type="time" onchange="setTransitionTime('Night', this.value)">
    </div>
    <br/>
    <div>
      Brightness:
      <input id="nightBrightness" type="range" min="0" max="100" value="50" onchange="setBrightness('Night', this.value)">
    </div>
    <br/>
    <div>
      Speed:
      <input id="nightMaxSpeed" type="range" min="0" max="100" value="25" onchange="setMaxSpeed('Night', this.value)">
    </div>
    <br/>
    <div id="animationNightSwitches"></div>
  </div>
</div>

<script>
var systemTimeAtPageLoad;

fetch('/getState')
  .then(response => response.json())
  .then(initializePage)
  .catch(error => console.error('Error fetching device state:', error));

function initializePage(deviceState) {
    // Dynamically generate switches for animations

    const animationDaySwitchesContainer = document.getElementById('animationDaySwitches');
    deviceState.animationsDay.forEach((animation) => {
    const divElement = document.createElement('div');
    divElement.innerHTML = `
        <label class="switch">
        <input type="checkbox" onchange="toggleAnimation('${animation.name}', this.checked, false)" ${animation.enabled ? 'checked' : ''}>
        <span class="slider rounded"></span>
        </label>
        <span class="label">${animation.name}</span>
    `;
    animationDaySwitchesContainer.appendChild(divElement);
    });

    const animationNightSwitchesContainer = document.getElementById('animationNightSwitches');
    deviceState.animationsNight.forEach((animation) => {
    const divElement = document.createElement('div');
    divElement.innerHTML = `
        <label class="switch">
        <input type="checkbox" onchange="toggleAnimation('${animation.name}', this.checked, true)" ${animation.enabled ? 'checked' : ''}>
        <span class="slider rounded"></span>
        </label>
        <span class="label">${animation.name}</span>
    `;
    animationNightSwitchesContainer.appendChild(divElement);
    });

    document.getElementById('transitionType').value = deviceState.transitionBehavior;
    document.getElementById('systemTime').innerText = deviceState.systemTime;
    document.getElementById('isNtpSynced').innerText = deviceState.isNtpSynced;
    document.getElementById('isNight').innerText = deviceState.isNight ? 'Night' : 'Day';
    document.getElementById('lightSensorReading').innerText = deviceState.lightSensorReading;
    document.getElementById('lightSensorThreshold').value = deviceState.lightSensorThreshold;
    document.getElementById('dayBrightness').value = deviceState.dayBrightness;
    document.getElementById('dayMaxSpeed').value = deviceState.dayMaxSpeed;
    document.getElementById('dayTransitionTime').value = deviceState.dayTransitionTime;
    document.getElementById('nightBrightness').value = deviceState.nightBrightness;
    document.getElementById('nightMaxSpeed').value = deviceState.nightMaxSpeed;
    document.getElementById('nightTransitionTime').value = deviceState.nightTransitionTime;
    document.getElementById(isNight ? 'nightBrightness' : 'dayBrightness').classList.add('active-slider');

    // used for running system clock
    const [hours, minutes, seconds] = deviceState.systemTime.split(":");
    systemTimeAtPageLoad = new Date();
    systemTimeAtPageLoad.setHours(hours);
    systemTimeAtPageLoad.setMinutes(minutes);
    systemTimeAtPageLoad.setSeconds(seconds);

    updateSystemTime();
    updateLightSensorReading();
}

function updateSystemTime() {
  const now = new Date();
  const elapsedTime = now - systemTimeAtPageLoad;
  const currentTime = new Date(systemTimeAtPageLoad.getTime() + elapsedTime);

  let h = currentTime.getHours().toString().padStart(2, '0');
  let m = currentTime.getMinutes().toString().padStart(2, '0');
  let s = currentTime.getSeconds().toString().padStart(2, '0');

  document.getElementById('systemTime').innerText = `${h}:${m}:${s}`;
  setTimeout(updateSystemTime, 1000);
}

// get plain text light sensor reading
function updateLightSensorReading() {
  fetch('/getLightSensorReading')
    .then(response => response.text())
    .then(reading => {
      document.getElementById('lightSensorReading').innerText = reading;
      setTimeout(updateLightSensorReading, 1000);
    })
    .catch(error => console.error('Error fetching light sensor reading:', error));
}

function toggleAnimation(animationName, isEnabled, isNight) {
  let action;
  if (isNight) {
    action = isEnabled ? 'enableAnimationNight' : 'disableAnimationNight';
  } else {
    action = isEnabled ? 'enableAnimationDay' : 'disableAnimationDay';
  }
  fetch(`/${action}?name=${animationName}`);
}

function restartDevice() {
  fetch('/restart');
}

function syncTime() {
  fetch('/syncTime');
}

function setBrightness(timeOfDay, brightness) {
  fetch(`/set${timeOfDay}Brightness?value=${brightness}`);
}

function setLightSensorThreshold(threshold) {
  fetch(`/setLightSensorThreshold?value=${threshold}`);
}

function setMaxSpeed(timeOfDay, speed) {
  fetch(`/set${timeOfDay}MaxSpeed?value=${speed}`);
}

function setTransitionTime(timeOfDay, time) {
  fetch(`/set${timeOfDay}TransitionTime?value=${time}`);
}

function setTransitionType(transitionType) {
  fetch(`/setTransitionType?value=${transitionType}`);
}

</script>
</body>                 
</html>
)====";