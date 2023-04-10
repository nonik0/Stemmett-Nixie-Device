//
// T6 | T5 | T4 | T3 | T2 | T1
//
#define EN_PIN 7      // OE pin
#define LATCH_PIN 34  // RCLK pin
#define PWM_PIN_1 1   // PWM pin for tube 1 anode
#define PWM_PIN_2 2   // PWM pin for tube  anode
#define PWM_PIN_3 3   // PWM pin for tube  anode
#define PWM_PIN_4 4   // PWM pin for tube  anode
#define PWM_PIN_5 5   // PWM pin for tube  anode
#define PWM_PIN_6 6   // PWM pin for tube  anode

#define NUM_TUBES 6

#define PWM_MAX 255
#define PWM_MIN 0
#define PWM_FREQUENCY 200 
#define PWM_RESOLUTION 8
#define REFRESH_RATE_US 1000  // 1ms

//|IN-4 |IN-7 |IN-7A|IN-7B|
//|  0  |  -  |     |     |
//|  1  |  П  |  M  |     |
//|  2  |  +  |  +  |  S  |
//|  3  |  m  |  m  |  ㎐ |
//|  4  |  A  |  %  |     |
//|  5  |  V  |     |     |
//|  6  |  M  |  µ  |  V  |
//|  7  |  Ω  |  -  |     |
//|  8  |  ~  |  к  |  Ω  |
//|  9  |  к  |  П  |     |

typedef enum {
  IN4 = 0,
  IN7 = 1,
  IN7A = 2,
  IN7B = 3,
} TubeType;

typedef enum {
  IN4_0 = 0,
  IN4_1 = 1,
  IN4_2 = 2,
  IN4_3 = 3,
  IN4_4 = 4,
  IN4_5 = 5,
  IN4_6 = 6,
  IN4_7 = 7,
  IN4_8 = 8,
  IN4_9 = 9,
} IN4_Cathodes;

typedef enum {
  IN7_Minus = 0,
  IN7_Pi = 1,
  IN7_Plus = 2,
  IN7_m = 3,
  IN7_A = 4,
  IN7_V = 5,
  IN7_M = 6,
  IN7_Omega = 7,
  IN7_Tilda = 8,
  IN7_Kappa = 9,
} IN7_Cathodes;

typedef enum {
  IN7A_M = 1,
  IN7A_Plus = 2,
  IN7A_m = 3,
  IN7A_Percent = 4,
  IN7A_Mu = 6,
  IN7A_Minus = 7,
  IN7A_Kappa = 8,
  IN7A_Pi = 9,
} IN7A_Cathodes;

typedef enum {
  IN7B_S = 2,
  IN7B_Hz = 3,
  IN7B_V = 6,
  IN7B_Omega = 8,
} IN7B_Cathodes;

const uint8_t TubeCathodeCount[4] = {10, 10, 8, 4};
const uint8_t TubeCathodes[4][10] = {
  {IN4_0, IN4_1, IN4_2, IN4_3, IN4_4, IN4_5, IN4_6, IN4_7, IN4_8, IN4_9},
  {IN7_Minus, IN7_Pi, IN7_Plus, IN7_m, IN7_A, IN7_V, IN7_M, IN7_Omega, IN7_Tilda, IN7_Kappa},
  {IN7A_M, IN7A_Plus, IN7A_m, IN7A_Percent, IN7A_Mu, IN7A_Minus, IN7A_Kappa, IN7A_Pi},
  {IN7B_S, IN7B_Hz, IN7B_V, IN7B_Omega}
};

typedef struct {
  const TubeType Type;
  const uint8_t PrimaryCathode;
  const uint8_t AnodePin;
  volatile int Delay;
  uint8_t ActiveCathode;
  uint8_t PwmPhase;
  // int PwmStart;
  // int PwmEnd;
} Tube;