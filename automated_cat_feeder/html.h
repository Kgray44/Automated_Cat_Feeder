// HTML Config Form
const char* configForm = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <title>Configure WiFi</title>
    <style>
      body { font-family: sans-serif; text-align: center; padding: 40px; }
      input { margin: 10px; padding: 8px; width: 200px; }
      button { padding: 10px 20px; }
    </style>
  </head>
  <body>
    <h2>Cat Feeder WiFi Setup</h2>
    <form action="/save" method="POST">
      <input name="ssid" placeholder="WiFi SSID"><br>
      <input name="password" type="password" placeholder="WiFi Password"><br>
      <button type="submit">Save & Reboot</button>
    </form>
  </body>
</html>
)rawliteral";

// HTML for Login Page
const char login_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Login</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
      background-color: #f4f4f4;
    }
    .container {
      background: #fff;
      padding: 20px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
      border-radius: 10px;
      text-align: center;
      width: 300px;
    }
    input[type="text"], input[type="password"] {
      width: 100%;
      padding: 10px;
      margin: 10px 0;
      border: 1px solid #ccc;
      border-radius: 5px;
    }
    button {
      padding: 10px 20px;
      font-size: 16px;
      background-color: #2196F3;
      color: white;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      transition: 0.3s;
    }
    button:hover {
      background-color: #1976D2;
    }
  </style>
</head>
<body>
  <div class="container">
    <h2>Login</h2>
    <form action="/login" method="post">
      <input type="text" name="username" placeholder="Username" required>
      <input type="password" name="password" placeholder="Password" required>
      <button type="submit">Login</button>
    </form>
  </div>
</body>
</html>
)rawliteral";

// HTML for Extras Page
const char extras_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Feeder Extras</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
      background-color: #f4f4f4;
    }
    .container {
      background: #fff;
      padding: 20px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
      border-radius: 10px;
      text-align: center;
      width: 300px;
    }
    button {
      padding: 10px 20px;
      font-size: 16px;
      background-color: #2196F3;
      color: white;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      transition: 0.3s;
    }
    button:hover {
      background-color: #1976D2;
    }
  </style>
</head>
<body>
  <div class="container">
    <h2>Extras</h2>
    <form action="/change" method="get">
      <button type="submit">Change Password</button>
    </form>
    <form action="/settings" method="get">
      <button type="submit">Settings</button>
    </form>
  </div>
</body>
</html>
)rawliteral";

// HTML for change password Page
const char change_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Change Password</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
      background-color: #f4f4f4;
    }
    .container {
      background: #fff;
      padding: 20px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
      border-radius: 10px;
      text-align: center;
      width: 300px;
    }
    input[type="password"] {
      width: 100%;
      padding: 10px;
      margin: 10px 0;
      border: 1px solid #ccc;
      border-radius: 5px;
    }
    button {
      padding: 10px 20px;
      font-size: 16px;
      background-color: #2196F3;
      color: white;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      transition: 0.3s;
    }
    button:hover {
      background-color: #1976D2;
    }
    .message {
      margin-top: 10px;
      color: #d32f2f;
    }
    .strength-meter {
      height: 10px;
      margin: 10px 0;
      background-color: #e0e0e0;
      border-radius: 5px;
      overflow: hidden;
    }
    .strength-meter div {
      height: 100%;
      transition: width 0.3s;
    }
    .strength-weak {
      background-color: #f44336;
    }
    .strength-medium {
      background-color: #ff9800;
    }
    .strength-strong {
      background-color: #4caf50;
    }
  </style>
  <script>
    function validateForm() {
      var password = document.forms["changePasswordForm"]["password"].value;
      var passwordd = document.forms["changePasswordForm"]["passwordd"].value;
      if (password != passwordd) {
        document.getElementById("message").innerHTML = "Passwords do not match.";
        return false;
      }
      return true;
    }

    function checkPasswordStrength() {
      var password = document.getElementById("password").value;
      var strengthMeter = document.getElementById("strengthMeter");
      var strength = 0;

      if (password.length >= 8) strength += 1;
      if (/[A-Z]/.test(password)) strength += 1;
      if (/[a-z]/.test(password)) strength += 1;
      if (/[0-9]/.test(password)) strength += 1;
      if (/[\W]/.test(password)) strength += 1;

      switch (strength) {
        case 1:
        case 2:
          strengthMeter.style.width = "20%";
          strengthMeter.className = "strength-weak";
          break;
        case 3:
          strengthMeter.style.width = "60%";
          strengthMeter.className = "strength-medium";
          break;
        case 4:
        case 5:
          strengthMeter.style.width = "100%";
          strengthMeter.className = "strength-strong";
          break;
        default:
          strengthMeter.style.width = "0";
          strengthMeter.className = "";
      }
    }

    document.addEventListener('DOMContentLoaded', function() {
      document.getElementById("changePasswordForm").onsubmit = submitForm;
      document.getElementById("password").oninput = checkPasswordStrength;
    });

    function submitForm(event) {
      event.preventDefault(); // Prevent form from submitting traditionally

      if (!validateForm()) {
        return; // If form validation fails, do not proceed
      }

      var formData = new FormData(document.forms["changePasswordForm"]);

      fetch('/save-change', {
        method: 'POST',
        body: formData
      })
      .then(response => response.text())
      .then(data => {
        document.getElementById("message").innerHTML = data; // Display the server response
      })
      .catch(error => {
        console.error('Error:', error);
        document.getElementById("message").innerHTML = "An error occurred while changing the password.";
      });
    }
  </script>
</head>
<body>
  <div class="container">
    <h2>Change Password</h2>
    <form id="changePasswordForm" method="post">
      <input type="password" id="password" name="password" placeholder="New Password" required oninput="checkPasswordStrength()">
      <div class="strength-meter">
        <div id="strengthMeter"></div>
      </div>
      <input type="password" id="passwordd" name="passwordd" placeholder="Confirm Password" required>
      <button type="submit">Change</button>
    </form>
    <div id="message" class="message"></div>
  </div>
</body>
</html>
)rawliteral";

const char settings_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Feeder Settings</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
      background-color: #f4f4f4;
    }
    .container {
      background: #fff;
      padding: 20px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
      border-radius: 10px;
      text-align: center;
      width: 300px;
    }
    label {
      display: block;
      margin-bottom: 10px;
      font-size: 16px;
      text-align: left;
    }
    input[type="number"] {
      width: 100%;
      padding: 10px;
      margin-bottom: 20px;
      border-radius: 5px;
      border: 1px solid #ccc;
      font-size: 16px;
    }
    button {
      padding: 10px 20px;
      font-size: 16px;
      background-color: #2196F3;
      color: white;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      transition: 0.3s;
    }
    button:hover {
      background-color: #1976D2;
    }
    #responseMessage {
      margin-top: 20px;
      font-size: 14px;
      color: green;
      display: none;
    }
  </style>
  <script>
  </script>

</head>
<body>
  <div class="container">
    <h2>Settings</h2>
    <form id="settingsForm">
      <label for="motorTime">Feeder Time Length (seconds):</label>
      <input type="number" id="motorTime" name="motorTime" min="1" max="30">
      <button type="submit">Save</button>
    </form>
  </div>
</body>
</html>
)rawliteral";


const char schedule_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Feeding Schedule</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 100vh;
      background-color: #f4f4f4;
      margin: 0;
    }
    .container {
      background: #fff;
      padding: 20px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
      border-radius: 10px;
      text-align: center;
      width: 90%;
      max-width: 600px;
    }
    h2 {
      margin-bottom: 20px;
    }
    table {
      width: 100%;
      border-collapse: collapse;
    }
    td, th {
      padding: 8px;
      text-align: center;
    }
    select {
      padding: 5px;
      margin: 4px;
      font-size: 14px;
    }
    button {
      padding: 10px 20px;
      margin-top: 20px;
      font-size: 16px;
      background-color: #2196F3;
      color: white;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      transition: 0.3s;
    }
    button:hover {
      background-color: #1976D2;
    }
    .time-slot {
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
      gap: 5px;
    }
  </style>
</head>
<body>
  <div class="container">
    <h2>Feeding Schedule</h2>
    <form id="scheduleForm">
      <table>
        <thead>
          <tr><th>Day</th><th>Feeding Times</th></tr>
        </thead>
        <tbody>
          %SCHEDULE_ROWS%
        </tbody>
      </table>
      <button type="submit">Save Schedule</button>
      <button type="button" onclick="resetSchedule()" style="margin-left: 10px; background-color: #f44336;">Reset Schedule</button>

    </form>
  </div>

  <script>
  </script>
</body>
</html>
)rawliteral";

//html page for resetting the device and erasing flash
const char manage_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Manage Device</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      background-color: #f4f4f4;
      margin: 0;
    }
    .container {
      background: #fff;
      padding: 20px;
      border-radius: 10px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
      text-align: center;
    }
    button {
      padding: 10px 20px;
      font-size: 16px;
      margin: 10px;
      border: none;
      border-radius: 5px;
      background-color: #2196F3;
      color: white;
      cursor: pointer;
      transition: 0.3s;
    }
    button:hover {
      background-color: #1976D2;
    }
  </style>
</head>
<body>
  <div class="container">
    <h2>Device Management</h2>
    <button onclick="resetMemory()" style="background-color: #f44336;">Erase All Saved Data</button>
    <button onclick="restartDevice()" style="background-color: #f44336;">Restart Device</button>
  </div>
  <script>
    function resetMemory() {
      if (confirm("Are you sure you want to erase all saved data? This cannot be undone.")) {
        fetch("/reset", { method: "POST" })
          .then(response => response.text())
          .then(msg => alert(msg))
          .catch(err => alert("Reset failed"));
      }
    }

    function restartDevice() {
      if (confirm("Restart the device now?")) {
        fetch("/restart", { method: "POST" })
          .then(response => response.text())
          .then(msg => alert(msg))
          .catch(err => alert("Restart failed"));
      }
    }
  </script>
</body>
</html>
)rawliteral";


// HTML for Main Control Page
// Updated HTML for Main Control Page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Cat Feeder Control</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
      background-color: #f4f4f4;
    }
    .container {
      background: #fff;
      padding: 20px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
      border-radius: 10px;
      text-align: center;
      width: 400px;
      position: relative;
    }
    h2 {
      margin-bottom: 20px;
      color: #333;
    }
    .toggle {
      margin: 15px 0;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    .led {
      height: 20px;
      width: 20px;
      background-color: red;
      border-radius: 50%;
      display: inline-block;
      margin-left: 10px;
    }
    .led.green {
      background-color: green;
    }
    .switch {
      position: relative;
      display: inline-block;
      width: 50px;
      height: 24px;
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
      transition: 0.4s;
      border-radius: 24px;
    }
    .slider:before {
      position: absolute;
      content: "";
      height: 16px;
      width: 16px;
      left: 4px;
      bottom: 4px;
      background-color: white;
      transition: 0.4s;
      border-radius: 50%;
    }
    input:checked + .slider {
      background-color: #2196F3;
    }
    input:checked + .slider:before {
      transform: translateX(26px);
    }
    button {
      padding: 10px 20px;
      margin-top: 20px;
      font-size: 16px;
      background-color: #2196F3;
      color: white;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      transition: 0.3s;
    }
    button:hover {
      background-color: #1976D2;
    }

    /* Menu Styles */
    nav.menu {
      background-color: #333;
      padding: 10px 20px;
      border-radius: 10px;
      margin-bottom: 20px;
    }

    nav.menu ul {
      list-style-type: none;
      margin: 0;
      padding: 0;
      display: flex;
      justify-content: space-around;
    }

    nav.menu ul li {
      position: relative;
    }

    nav.menu ul li a {
      color: #fff;
      padding: 10px 20px;
      text-decoration: none;
      display: block;
      transition: background-color 0.3s ease, color 0.3s ease;
      border-radius: 5px;
    }

    nav.menu ul li a:hover {
      background-color: #555;
      color: #00bcd4; /* Accent color */
    }

    /* Dropdown Styles */
    nav.menu ul li.dropdown ul.dropdown-content {
      display: none;
      position: absolute;
      top: 100%;
      left: 0;
      background-color: #444;
      box-shadow: 0px 8px 16px rgba(0, 0, 0, 0.2);
      z-index: 1;
      border-radius: 5px;
    }

    nav.menu ul li.dropdown:hover ul.dropdown-content {
      display: block;
      animation: fadeIn 0.3s ease-in-out;
    }

    nav.menu ul li.dropdown ul.dropdown-content li {
      width: 200px;
    }

    nav.menu ul li.dropdown ul.dropdown-content li a {
      padding: 10px;
      text-align: left;
      background-color: #555;
      font-size: 14px;
      transition: background-color 0.3s ease, color 0.3s ease;
      border-radius: 5px;
    }

    nav.menu ul li.dropdown ul.dropdown-content li a:hover {
      background-color: #666;
      color: #00bcd4;
    }

    /* Animations */
    @keyframes fadeIn {
      from {
        opacity: 0;
        transform: translateY(-10px);
      }
      to {
        opacity: 1;
        transform: translateY(0);
      }
    }

    /* Dynamic Content Styles */
    #content {
      margin-top: 20px;
      padding: 20px;
      background-color: #f9f9f9;
      box-shadow: 0 0 15px rgba(0, 0, 0, 0.1);
      border-radius: 10px;
      opacity: 0;
      transform: translateY(20px);
      transition: opacity 0.3s ease, transform 0.3s ease;
    }
    #content.show {
      opacity: 1;
      transform: translateY(0);
    }
  </style>
  <script>
    var gateway = `ws://${window.location.hostname}/ws`;
    var websocket;

    window.addEventListener('load', onLoad);

    function onLoad(event) {
      initWebSocket();
    }

    function initWebSocket() {
      console.log('Trying to open a WebSocket connection...');
      websocket = new WebSocket(gateway);
      websocket.onopen    = onOpen;
      websocket.onclose   = onClose;
      websocket.onmessage = onMessage;
    }

    function onOpen(event) {
      console.log('Connection opened');
      websocket.send('getStatus');
    }

    function onClose(event) {
      console.log('Connection closed');
      setTimeout(initWebSocket, 2000);
    }

    function onMessage(event) {
      var data = JSON.parse(event.data);  // Parse the incoming JSON data
      document.getElementById('automated').checked = data.automated;
      document.getElementById('cat_recognition').checked = data.cat_recognition;

      var feedStatus = document.getElementById('feedStatus');  // Get the LED element by its ID

      if (data.motorOn) {  // If the feeder is on
        feedStatus.classList.remove('red');   // Remove the 'red' class
        feedStatus.classList.add('green');    // Add the 'green' class
      } else {  // If the door is locked
        feedStatus.classList.remove('green'); // Remove the 'green' class
        feedStatus.classList.add('red');      // Add the 'red' class
      }

      const motorCurrentElem = document.getElementById("motorCurrentDisplay");

      if (data.motorOn) {
        // Show current and colorize based on value
        motorCurrentElem.style.display = "block";
        motorCurrentElem.textContent = `Current: ${data.current} mA`;

        const current = parseFloat(data.current);
        if (current < 300) motorCurrentElem.style.color = "#4CAF50"; // green
        else if (current < 450) motorCurrentElem.style.color = "#FF9800"; // orange
        else motorCurrentElem.style.color = "#F44336"; // red

      } else {
        // Hide when motor is off
        motorCurrentElem.style.display = "none";
      }
    }

    function toggleFunction(id) {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/" + id + "?state=" + (document.getElementById(id).checked ? "1" : "0"), true);
      xhr.send();
    }

    function feederOn() {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/feed", true);
      xhr.send();
    }

    document.addEventListener('DOMContentLoaded', function() {
    const menuLinks = document.querySelectorAll('.menu-link');

    menuLinks.forEach(link => {
      link.addEventListener('click', function(e) {
        e.preventDefault();
        const page = this.getAttribute('data-page');
        loadPage(page);
      });
    });
  });

  function loadPage(page) {
    if (page === 'home') {
      document.getElementById('content').innerHTML = '';
      content.classList.remove('show');
    }
    fetch('/' + page)
      .then(response => response.text())
      .then(data => {
        document.getElementById('content').innerHTML = data;
        content.classList.add('show');

        // Re-attach form and button logic for specific pages
        if (page === 'change') attachPasswordFormListeners();
        else if (page === 'settings') attachSettingsFormListener();
        else if (page === 'schedule') attachScheduleFormListener();
        else if (page === 'manage') attachManageButtons();
      })
      .catch(error => console.error('Error loading page:', error));
  }

  function attachPasswordFormListeners() {
    document.forms["changePasswordForm"].onsubmit = validateForm;
    document.getElementById("password").oninput = checkPasswordStrength;
  }

  function attachManageButtons() {
    const resetBtn = document.querySelector('button[onclick="resetMemory()"]');
    const restartBtn = document.querySelector('button[onclick="restartDevice()"]');

    if (resetBtn) {
      resetBtn.onclick = function () {
        if (confirm("Are you sure you want to erase all saved data? This cannot be undone.")) {
          fetch("/reset", { method: "POST" })
            .then(response => response.text())
            .then(msg => alert(msg))
            .catch(err => alert("Reset failed"));
        }
      };
    }

    if (restartBtn) {
      restartBtn.onclick = function () {
        if (confirm("Restart the device now?")) {
          fetch("/restart", { method: "POST" })
            .then(response => response.text())
            .then(msg => alert(msg))
            .catch(err => alert("Restart failed"));
        }
      };
    }
  }
  function attachSettingsFormListener() {
    console.log("Settings form JS loaded");

    fetch("/api/motortime")
      .then(res => res.json())
      .then(data => {
        document.getElementById("motorTime").value = data.motorTime;
      });

    const form = document.getElementById("settingsForm");
    if (!form) return;

    form.onsubmit = function (e) {
      e.preventDefault();
      let formData = new FormData(form);

      fetch("/save-settings", {
        method: "POST",
        body: formData,
        credentials: 'same-origin'
      })
      .then(response => {
        if (response.ok) return response.text();
        else throw new Error("Error: " + response.status);
      })
      .then(data => alert(data))  // ✅ Show success message
      .catch(error => alert(error.message));
    };
  }

  function attachScheduleFormListener() {
    fetch("/api/schedule")
      .then(res => res.json())
      .then(schedule => {
        const days = ["Sun","Mon","Tue","Wed","Thu","Fri","Sat"];
        days.forEach(day => {
          if (schedule[day]) {
            for (let i = 0; i < schedule[day].length; i++) {
              const input = document.getElementById(day + i);
              if (input) input.value = schedule[day][i];
            }
          }
        });
      });

    document.getElementById("scheduleForm").addEventListener("submit", function(e) {
      e.preventDefault();

      const data = {};
      const days = ["Sun","Mon","Tue","Wed","Thu","Fri","Sat"];

      days.forEach(day => {
        data[day] = [];
        for (let i = 0; i < 4; i++) {
          let val = document.getElementById(day + i).value;
          if (val !== "") data[day].push(val);
        }
      });

      fetch("/save-schedule", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(data),
        credentials: "same-origin"
      })
      .then(res => res.text())
      .then(msg => alert(msg))
      .catch(err => alert("Error saving schedule"));
    });

    // Bind resetSchedule button if present
    const resetBtn = document.querySelector('button[onclick="resetSchedule()"]');
    if (resetBtn) {
      resetBtn.onclick = function () {
        if (!confirm("Are you sure you want to reset the entire schedule?")) return;

        fetch("/reset-schedule", {
          method: "POST",
          credentials: "same-origin"
        })
        .then(res => res.text())
        .then(msg => {
          alert(msg);
          location.reload(); // Reload to reflect cleared inputs
        })
        .catch(err => alert("Error resetting schedule"));
      };
    }
  }
  

  function validateForm() {
    var password = document.forms["changePasswordForm"]["password"].value;
    var passwordd = document.forms["changePasswordForm"]["passwordd"].value;
    if (password != passwordd) {
      document.getElementById("message").innerHTML = "Passwords do not match.";
      return false;
    }
    return true;
  }

  function checkPasswordStrength() {
    var password = document.getElementById("password").value;
    var strengthMeter = document.getElementById("strengthMeter");
    var strength = 0;

    if (password.length >= 8) strength += 1;
    if (/[A-Z]/.test(password)) strength += 1;
    if (/[a-z]/.test(password)) strength += 1;
    if (/[0-9]/.test(password)) strength += 1;
    if (/[\W]/.test(password)) strength += 1;

    switch (strength) {
      case 1:
      case 2:
        strengthMeter.style.width = "20%";
        strengthMeter.className = "strength-weak";
        break;
      case 3:
        strengthMeter.style.width = "60%";
        strengthMeter.className = "strength-medium";
        break;
      case 4:
      case 5:
        strengthMeter.style.width = "100%";
        strengthMeter.className = "strength-strong";
        break;
      default:
        strengthMeter.style.width = "0";
        strengthMeter.className = "";
    }
  }
  </script>
</head>
<body>
  <div class="container">
    <h2>Cat Feeder Control</h2>

    <nav class="menu">
      <ul>
        <li><a href="#" class="menu-link" data-page="home">Home</a></li>
        <li class="dropdown">
          <a href="#" class="menu-link" data-page="extras">Extras</a>
          <ul class="dropdown-content">
            <li><a href="#" class="menu-link" data-page="change">Change Password</a></li>
            <li><a href="#" class="menu-link" data-page="settings">Change Settings</a></li>
            <li><a href="#" class="menu-link" data-page="schedule">Schedule</a></li>
            <li><a href="#" class="menu-link" data-page="manage">Manage Device</a></li>
          </ul>
        </li>
        <li><a href="#" class="menu-link" data-page="ota">Update OS</a></li>
      </ul>
    </nav>

    <div class="toggle">
      <label for="automated">Enable Automated</label>
      <label class="switch">
        <input type="checkbox" id="automated" onchange="toggleFunction('automated')">
        <span class="slider"></span>
        </label>
    </div>

    <div class="toggle">
      <label for="cat_recognition">Enable Cat Recognition</label>
      <label class="switch">
        <input type="checkbox" id="cat_recognition" onchange="toggleFunction('cat_recognition')">
        <span class="slider"></span>
      </label>
    </div>

    <button onclick="feederOn()">Feed Now</button>

    <div id="feedStatus" class="led red"></div>

    <div id="motorCurrentDisplay" style="margin-top:10px; font-size:14px; font-weight:bold; display:none;">
      Current: -- mA
    </div>
  </div>
  
  <div id="content">
    <!-- Dynamic content will be loaded here -->
  </div>
</body>
</html>
)rawliteral";
//  document.addEventListener("DOMContentLoaded", attachScheduleFormListener);


// HTML for OTA Update Page
const char update_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>OTA Update</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
    }
    .container {
      text-align: center;
      background: #fff;
      padding: 20px;
      border-radius: 10px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
    }
    input[type=file] {
      padding: 10px;
      margin: 20px 0;
    }
    button {
      padding: 10px 20px;
      font-size: 16px;
    }
  </style>
</head>
<body>
  <div class="container">
    <h2>Upload New Firmware</h2>
    <form method="POST" action="/update" enctype="multipart/form-data">
      <input type="file" name="firmware" accept=".bin"><br>
      <button type="submit">Upload</button>
    </form>
  </div>
</body>
</html>
)rawliteral";
