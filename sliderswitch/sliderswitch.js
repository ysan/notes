class SliderSwitch {
  constructor(idBg, idBall, cbCheck, init = false, size = 1) {
    this.idBg = idBg;
    this.idBall = idBall;
    this.cbCheck = cbCheck;
    this.current = init;
    if (size < 0) {
      this.size = 0;
    } else if (size > 2) {
      this.size = 2;
    } else {
      this.size = size;
    }

    this.initializ();
  }

  sizeString = ['_small', '', '_big'];
  getSizeString = () => {
    return this.sizeString[this.size];
  };

  initializ = () => {
    if (this.current) {
      this.enable();
    }

    const elemBg = document.getElementById(this.idBg);
    if (elemBg) {
      elemBg.classList.add('sliderswitch_bg' + this.getSizeString());
      elemBg.addEventListener('click', () => {
        if (this.cbCheck) {
          this.cbCheck(this);
        }
      });
    }

    const elemBall = document.getElementById(this.idBall);
    if (elemBall) {
      elemBall.classList.add('sliderswitch_ball' + this.getSizeString());
    }
  };

  enable() {
    const elemBg = document.getElementById(this.idBg);
    if (elemBg) {
      if (!elemBg.classList.contains('sliderswitch_bg__enabled')) {
        elemBg.classList.add('sliderswitch_bg__enabled');
      }
    }

    const elemBall = document.getElementById(this.idBall);
    if (elemBall) {
      if (
        elemBall.classList.contains(
          'sliderswitch_ball_to_disable' + this.getSizeString()
        )
      ) {
        elemBall.classList.remove(
          'sliderswitch_ball_to_disable' + this.getSizeString()
        );
      }
      if (
        !elemBall.classList.contains(
          'sliderswitch_ball_to_enable' + this.getSizeString()
        )
      ) {
        elemBall.classList.add(
          'sliderswitch_ball_to_enable' + this.getSizeString()
        );
      }
    }

    this.current = true;
  }

  disable = () => {
    const elemBg = document.getElementById(this.idBg);
    if (elemBg) {
      if (elemBg.classList.contains('sliderswitch_bg__enabled')) {
        elemBg.classList.remove('sliderswitch_bg__enabled');
      }
    }

    const elemBall = document.getElementById(this.idBall);
    if (elemBall) {
      if (
        elemBall.classList.contains(
          'sliderswitch_ball_to_enable' + this.getSizeString()
        )
      ) {
        elemBall.classList.remove(
          'sliderswitch_ball_to_enable' + this.getSizeString()
        );
      }
      if (
        !elemBall.classList.contains(
          'sliderswitch_ball_to_disable' + this.getSizeString()
        )
      ) {
        elemBall.classList.add(
          'sliderswitch_ball_to_disable' + this.getSizeString()
        );
      }
    }

    this.current = false;
  };

  switch = () => {
    if (this.current) {
      this.disable();
    } else {
      this.enable();
    }
  };
}
