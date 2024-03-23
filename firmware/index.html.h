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
    height: 34px;
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
    height: 26px;
    width: 26px;
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
    -webkit-transform: translateX(26px);
    -ms-transform: translateX(26px);
    transform: translateX(26px);
  }
  .slider.rounded {
    border-radius: 10px;
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
    <p>Animations:</p>
    <div id="animationSwitches"></div>
  </div>  
  <div>
    <p>Animation Transition Type:</p>
    <select id="transitionType" onchange="setTransitionType(this.value)">
      <option value="random">Random</option>
      <option value="sequential">Sequential</option>
    </select>
  </div>
  
  <div>
    <p>Daytime Brightness:</p>
    <input type="range" min="0" max="100" value="50" oninput="setBrightness('Day', this.value)">
  </div>
  <div>
    <p>Nighttime Brightness:</p>
    <input type="range" min="0" max="100" value="50" oninput="setBrightness('Night', this.value)">
  </div>
  
  <!-- Time selectors for transitions -->
  <div>
    <p>Day Start Time:</p>
    <input type="time" id="dayStartTime" onchange="setTransitionTime('Day', this.value)">
  </div>
  <div>
    <p>Night Start Time:</p>
    <input type="time" id="nightStartTime" onchange="setTransitionTime('Night', this.value)">
  </div>
  
</div>

<script>
const AnimationTypes = [
  "BasicFade",
  "Bubble",
  "CylonScan",
  "Ghost",
  "NameGlitch",
  "Pulsar",
  "Rain",
  "RandomScan",
  "Scan",
  "SlotMachine",
];

// Dynamically generate switches for animations
const animationSwitchesContainer = document.getElementById('animationSwitches');
AnimationTypes.forEach((animation) => {
  const divElement = document.createElement('div');
  divElement.innerHTML = `
    <label class="switch">
      <input type="checkbox" onchange="toggleAnimation('${animation}', this.checked)">
      <span class="slider rounded"></span>
      </label>
    <span class="label">${animation}</span>
  `;
  animationSwitchesContainer.appendChild(divElement);
});

function toggleAnimation(animationName, isEnabled) {
  const action = isEnabled ? 'enableAnimation' : 'disableAnimation';
  fetch(`/${action}?name=${animationName}`);
}

function restartDevice() {
  fetch('/restart');
}

function syncTime() {
  fetch('/syncTime');
}

function setTransitionType(transitionType) {
  fetch(`/setTransitionType?type=${transitionType}`);
}

function setBrightness(time, brightness) {
  fetch(`/set${time}Brightness?value=${brightness}`);
}

</script>
</body>
</html>
)====";