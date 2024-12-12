/*
 * the following adapted from
 * https://community.home-assistant.io/t/share-your-esphome-light-effects/250294/14
 */
#include <math.h>

#include "esphome.h"

static void red_green_rainbow_select_next_direction(void)
{
  id(red_green_rainbow_direction).make_call().select_next(true).perform();
}

static void red_green_rainbow(AddressableLight &it,
                              const Color &current_color,
                              bool initial_run)
{
  static float phase = 0.0;
  const float phase_increment = (2.0 * M_PI *
                                 (id(addressable_lambda_update_interval_ms) /
                                  1000.0) /  /* at 1 Hz here */
                                 id(red_green_rainbow_speed).state);

  static unsigned int cycle_number = 0;
  const unsigned int switch_cycles =
    (int(id(red_green_rainbow_direction_switch_interval).state) *
     1000 /  /* milliseconds per interval here */
     id(addressable_lambda_update_interval_ms)  /* divide by ms/cycle */);

  cycle_number += 1;
  if (switch_cycles > 0 &&
      cycle_number > switch_cycles) {
    red_green_rainbow_select_next_direction();
    cycle_number = 1;
  }

  for (auto i = 0; i < it.size(); i++) {
    float position = (float) i / it.size();
    float red_intensity = (sin(position * 2.0 * M_PI + phase) + 1.0) / 2.0;
    float green_intensity = (sin(position * 2.0 * M_PI + phase + M_PI)
                             + 1.0) / 2.0;

    it[i] = Color(
                  (uint8_t) (red_intensity * 255),
                  (uint8_t) (green_intensity * 255),
                  0
                  );
  }

  optional<size_t> active_index =
    id(red_green_rainbow_direction).active_index();
  if (active_index && *active_index == 1) {  /* reverse */
    phase += phase_increment;
    if (phase >= 2.0 * M_PI) {
      phase -= 2.0 * M_PI;
    }
  } else {
    phase -= phase_increment;
    if (phase < 0) {
      phase += 2.0 * M_PI;
    }
  }
}

static void red_green_twinkle(AddressableLight &it,
                              const Color &_,
                              bool initial_run)
{
  /* using float in case RAND_MAX is close to INT_MAX (to avoid overflow */
  float twinkle_rand_chance =
    (float(RAND_MAX) /
     id(addressable_lambda_update_frequency) /  /* 1 Hz AVERAGE / pixel here */
     id(red_green_twinkle_update_average).state);
  for (auto i = 0; i < it.size(); i++) {
    /* Randomly select some LEDs to twinkle */
    if (rand() < twinkle_rand_chance) {
      auto current_color = it[i].get();
      /* WARNING: gamma correction may stomp the exact values below */
      if (current_color.red > current_color.green) {
        it[i] = Color(0, 255, 0);  // Change from red to green
      } else {
        it[i] = Color(255, 0, 0);  // Change from green to red
      }
    }
  }
}

static void red_green_rainbow_twinkle_cycle(AddressableLight &it,
                                            const Color &current_color,
                                            bool initial_run)
{
  static unsigned int cycle_number = 0;
  const unsigned int rollover_cycles =
    (int(id(red_green_rainbow_twinkle_cycle_interval).state) *
     1000 /  /* milliseconds per interval here */
     id(addressable_lambda_update_interval_ms)  /* divide by ms/cycle */);

  if (initial_run) {
    cycle_number = 0;
  }

  cycle_number += 1;
  if (cycle_number > (rollover_cycles * 2)) {
    cycle_number = 1;
  }
  if (cycle_number <= rollover_cycles) {
    red_green_rainbow(it, current_color, initial_run);
  } else {
    red_green_twinkle(it, current_color, initial_run);
  }
}
