#include "unity.h"
#include "AS3935.h"
#include <stdio.h>
#include <string.h>
#include "mock_test_AS3935.h" //Indica que necesitas un mock

//Redefiniciones
int spi_read_register(struct SystemState *state, uint8_t reg, uint8_t *value) {
    if (reg == CONFIG_REG_3A || reg == CONFIG_REG_3B) {
        *value = 0x80;
    } else if (reg == CONFIG_REG_3) {
        *value = 0x00;
    }
    return 0;
}

int spi_write_register(struct SystemState *state, uint8_t reg, uint8_t value) {
    return 0;
}

void setUp(void) {
    // Se ejecuta antes de cada test
}

void tearDown(void) {
    // Se ejecuta después de cada test
}

void test_as3935_init_success(void) {
    struct SystemState state;
    memset(&state, 0, sizeof(state));

    //Siempre que el cçodigo tenga que llamar a una función mockeada, se debe configurar mock en el test
    //Como en este caso, as3935_fopen no es crítica, pero hay que configurar el mock
    as3935_fopen_IgnoreAndReturn(tmpfile()); 
    log_timestamp_Ignore();
    int result = as3935_init(&state);

    TEST_ASSERT_EQUAL(0, result);
}

void test_as3935_init_log_file_already_open(void) {
    struct SystemState state;
    memset(&state, 0, sizeof(state));
    
    state.log_file = tmpfile(); // Crea un archivo temporal para simular el log
    TEST_ASSERT_NOT_NULL(state.log_file);

    log_timestamp_Ignore();

    int result = as3935_init(&state);

    TEST_ASSERT_EQUAL(0, result);

    fclose(state.log_file);
}

void test_as3935_init_log_file_open_fails(void) {
    struct SystemState state;
    memset(&state, 0, sizeof(state));

    as3935_fopen_IgnoreAndReturn(NULL);

    log_timestamp_Ignore();

    int result = as3935_init(&state);

    TEST_ASSERT_EQUAL(-1, result);
}