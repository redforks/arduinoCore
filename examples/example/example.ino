#include <core.h>

using namespace core::store;
using namespace core::clock;

void f() {
}

void setup() {
  defineDigital();
  defineAnalog();

  monitorDigitals(&f, 1, 1);
  monitorAnalogs(&f, 1, 1);

  beginBatchUpdate();
  setDigital(1, HIGH);
  setAnalog(1, 100);
  endBatchUpdate();

  removeInterval(interval(30, &f));
  removeDelay(delay(30, &f));
}

void loop() {
  check();
}
