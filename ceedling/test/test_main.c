#include "unity.h"
#include "app.h"

void setUp(void) {
    // Se ejecuta antes de cada test
}

void tearDown(void) {
    // Se ejecuta después de cada test
}

void test_algo_basico_de_app(void) {
    int result = funcion_de_app_que_quieras_testear();  // cambia esto por una real
    TEST_ASSERT_EQUAL(esperado, result);                // define el valor esperado
}
