#include "core.h"
#include <stdarg.h>

namespace core {

int compareULong(unsigned long a, unsigned long b, unsigned long flag) {
  unsigned long c = a - b;
  if (c == 0) {
    return 0;
  }

  if (c > flag) {
    return -1;
  }
  
  return 1;
}

struct Node {
  callback f;
  Node *next;

  unsigned long mills, interval;  // only used in clock namespace

  Node(callback fn) : Node(fn, 0) {}
  Node(callback fn, unsigned long delay) : Node(fn, delay, 0) {}
  Node(callback fn, unsigned long delay, unsigned long anInterval)
      : f(fn), next(NULL), mills(delay), interval(anInterval) {}
};

struct Queue {
  Node *head;

  void add(Node *);
  void clear();
  void invokeAll();
  void appendQueue(Queue);
  void remove(Node *);

  Queue() : head(NULL) {}

 private:
  bool exist(callback f);
};

void Queue::add(Node *node) {
  node->next = head;
  head = node;
}

void Queue::clear() {
  for (Node *p = head; p != NULL;) {
    Node *next = p->next;
    delete p;
    p = next;
  }
  head = NULL;
}

void Queue::invokeAll() {
  for (Node *p = head; p != NULL; p = p->next) {
    p->f();
  }
}

// append callbacks, ignored if it exist in Queue.
void Queue::appendQueue(Queue queue) {
  for (Node *p = queue.head; p != NULL; p = p->next) {
    if (!Queue::exist(p->f)) {
      add(new Node(p->f));
    }
  }
}

bool Queue::exist(callback f) {
  for (Node *p = head; p != NULL; p = p->next) {
    if (p->f == f) {
      return true;
    }
  }
  return false;
}

void Queue::remove(Node *node) {
  for (Node *p = head, *prev = NULL; p != NULL; prev = p, p = p->next) {
    if (p == node) {
      if (prev == NULL) {
        head = p->next;
      } else {
        prev->next = p->next;
        delete node;
        break;
      }
    }
  }
}

namespace store {
// Current value of digitals.
byte digitals[DIGITAL_VALUES];

// Current value of the analogs.
word analogs[ANALOG_VALUES];
Queue digitalMonitors[DIGITAL_VALUES];
Queue analogMonitors[ANALOG_VALUES];

idType digitalIdSeed = 0;
idType analogIdSeed = 0;

idType defineDigital() { return digitalIdSeed++; }

idType defineAnalog() { return analogIdSeed++; }

void monitorDigitals(callback f, byte nIds, ...) {
  va_list ap;
  va_start(ap, nIds);
  for (int i = 0; i < nIds; i++) {
    idType id = va_arg(ap, int);
    Node *node = new Node(f);
    store::digitalMonitors[id].add(node);
  }
}

void monitorAnalogs(callback f, byte nIds, ...) {
  va_list ap;
  va_start(ap, nIds);
  for (int i = 0; i < nIds; i++) {
    idType id = va_arg(ap, int);
    Node *node = new Node(f);
    store::analogMonitors[id].add(node);
  }
}

byte inBatch = 0;
Queue dirtyCallbacks;  // callbacks need trigger when batch end.
void beginBatchUpdate() { inBatch++; }

void endBatchUpdate() {
  inBatch--;
  if (inBatch == 0) {
    dirtyCallbacks.invokeAll();
    dirtyCallbacks.clear();
  }
}

void setDigital(idType id, byte val) {
  if (digitals[id] == val) {
    return;
  }

  digitals[id] = val;

  if (inBatch == 0) {
    digitalMonitors[id].invokeAll();
    return;
  }

  dirtyCallbacks.appendQueue(digitalMonitors[id]);
}

void setAnalog(idType id, word val) {
  if (analogs[id] == val) {
    return;
  }

  analogs[id] = val;

  if (inBatch == 0) {
    analogMonitors[id].invokeAll();
    return;
  }

  dirtyCallbacks.appendQueue(analogMonitors[id]);
}
}

namespace clock {

Queue intervals, delays;

void *interval(unsigned long mills, callback f) {
  Node *node = new Node(f, mills + millis(), mills);
  intervals.add(node);
  return node;
}

void *delay(unsigned long mills, callback f) {
  Node *node = new Node(f, mills + millis());
  delays.add(node);
  return node;
}

void removeInterval(void *id) { intervals.remove((Node *)id); }

void removeDelay(void *id) { delays.remove((Node *)id); }

void check() {
  unsigned long cur = millis();
  for (Node *p = delays.head; p != NULL; p = p->next) {
    if (compareULong(cur, p->mills, (unsigned long)(10) * 24 * 3600 * 1000) >= 0) {
      Serial.println("run a delay");
      delays.remove(p);
      p->f();
    }
  }

  for (Node *p = intervals.head; p != NULL; p = p->next) {
    if (compareULong(cur, p->mills, (unsigned long)(10) * 24 * 3600 * 1000) >= 0) {
      Serial.print("run a interval: ");
      Serial.print(cur);
      Serial.print(", ");
      Serial.println(p->mills);
      p->mills = cur + p->interval;
      p->f();
    }
  }
}
}
}
