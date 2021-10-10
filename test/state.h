#ifndef TEST_STATE_H
#define TEST_STATE_H

enum class State {
  none,
  default_constructed,
  constructed,
  copy_constructed,
  move_constructed,
  copy_assigned,
  move_assigned,
  destructed
};

#endif
