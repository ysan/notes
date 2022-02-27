window.onload = function () {
  document.getElementById('test_button').addEventListener('click', () => {
    //document.getElementById('modal_waiting_bg').style.display = 'block';
    //document.getElementById('modal_waiting_bg').style.opacity = 1;
    document.getElementById('modal_waiting_bg').style.visibility = 'visible';
    document.getElementById('modal_waiting_bg').style.opacity = 1;

    setTimeout(() => {
      document.getElementById('modal_waiting_bg').style.opacity = 0;
      setTimeout(() => {
        //document.getElementById('modal_waiting_bg').style.display = 'none';
        document.getElementById('modal_waiting_bg').style.visibility = 'hidden';
      }, 300);
    }, 3000);
  });
};
