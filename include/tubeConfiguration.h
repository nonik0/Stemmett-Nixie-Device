#if defined(STELLA)
Tube Tubes[NUM_TUBES] = {
            {IN7,  IN7_A    /*A*/, PWM_PIN_1, 0, 0, 0},
          {IN4,  IN4_7    /*L*/, PWM_PIN_2, 0, 0, 0},
        {IN4,  IN4_7    /*L*/, PWM_PIN_3, 0, 0, 0},
      {IN7A, IN7_m    /*E*/, PWM_PIN_4, 0, 0, 0},
    {IN7A, IN7_Plus /*T*/, PWM_PIN_5, 0, 0, 0},
  {IN7B, IN7B_S   /*S*/, PWM_PIN_6, 0, 0, 0},
};
#elif defined(EMMETT)
Tube Tubes[NUM_TUBES] = {
            {IN7, IN7_Plus /*T*/, PWM_PIN_1, 0, 0, 0},
          {IN7, IN7_Plus /*T*/, PWM_PIN_2, 0, 0, 0},
        {IN7A, IN7_m    /*E*/, PWM_PIN_3, 0, 0, 0},
      {IN7 , IN7_m    /*M*/, PWM_PIN_4, 0, 0, 0},
    {IN7 , IN7_m    /*M*/, PWM_PIN_5, 0, 0, 0},
  {IN7A, IN7_m    /*E*/, PWM_PIN_6, 0, 0, 0},
};
#endif