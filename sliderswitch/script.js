window.onload = function () {
  const ss = new SliderSwitch('ss_bg', 'ss_ball', check);
  const ss2 = new SliderSwitch('ss_bg2', 'ss_ball2', check, true, 0);
  const ss3 = new SliderSwitch('ss_bg3', 'ss_ball3', check, true, 2);
};

function check(e) {
  setTimeout(() => {
    if (e.current) {
      console.log('check to disable');
    } else {
      console.log('check to enable');
    }
    e.switch();
  }, 500);
}
