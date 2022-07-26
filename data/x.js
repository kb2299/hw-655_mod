const button_light = document.getElementById('button_light1');
const debug_info = document.getElementById('debug');
var timer1;

function status_update(data) {
    console.log(data)

    button_light.value = data.relay ? "On" : "Off";
    button_light.checked = data.relay ? true : false;
    button_light.disabled = false;
    debug_info.innerHTML = JSON.stringify(data)
    // if (timeoutID !== 'undefined') clearTimeout(timeoutID);
    // timeoutID = setTimeout(getStatus, 2000);
}

async function getStatus() {
    try {
        let response = await fetch('/status');
        let data = await response.json();
        status_update(data)
    } catch (error) {
        console.error(error);
    }
}

async function lightSwitch() {
    var result;
    button_light.disabled = true;
    if (button_light.value === 'On') {
        let result = await fetch('/off');


    } else {
        let result = await fetch('/on');
    }
    // getStatus()
}

button_light.addEventListener('click', lightSwitch);

getStatus()
timer1 = setInterval(getStatus,2000)


