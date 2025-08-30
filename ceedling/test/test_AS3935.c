#include "unity.h"
#include "AS3935.h"
#include <stdio.h>
#include <string.h>
#include "mock_test_AS3935.h" //Indica que necesitas un mock

static int simulate_write_preset_failure = 0;
static int simulate_write_calib_failure = 0;
static int simulate_write_0_failure = 0;
static int simulate_write_1_failure = 0;
static int simulate_write_2_failure = 0;
static int simulate_write_3_failure = 0;
static int simulate_read_3a_failure = 0;
static int simulate_read_3b_failure = 0;
static int simulate_read_3_failure = 0;
static uint8_t simulated_calib_trco_status = 0x80;
static uint8_t simulated_calib_srco_status = 0x80;

//Redefiniciones
int spi_read_register(struct SystemState *state, uint8_t reg, uint8_t *value) {
    if (simulate_read_3a_failure && reg == CONFIG_REG_3A) {
        return -1; 
    }
    if (simulate_read_3b_failure && reg == CONFIG_REG_3B) {
        return -1; 
    }
    if (simulate_read_3_failure && reg == CONFIG_REG_3) {
        return -1; 
    }
    if (reg == CONFIG_REG_3A) {
        *value = simulated_calib_trco_status;
    } else if (reg == CONFIG_REG_3B) {
        *value = simulated_calib_srco_status;
    } else {
        *value = 0x00;
    }
    return 0;
}

int spi_write_register(struct SystemState *state, uint8_t reg, uint8_t value) {
    if (simulate_write_preset_failure && reg == REG_PRESET_DEFAULT) {
        return -1; 
    }
    if (simulate_write_calib_failure && reg == REG_CALIB_RCO) {
        return -1; 
    }
    if (simulate_write_0_failure && reg == CONFIG_REG_0) {
        return -1;
    }
    if (simulate_write_1_failure && reg == CONFIG_REG_1) {
        return -1;
    }
    if (simulate_write_2_failure && reg == CONFIG_REG_2) {
        return -1;
    }
    if (simulate_write_3_failure && reg == CONFIG_REG_3) {
        return -1;
    }
    return 0; 
}

void setUp(void) {
    simulate_write_preset_failure = 0;
    simulate_write_calib_failure = 0;
    simulate_read_3a_failure = 0;
    simulate_read_3b_failure = 0;
    simulate_read_3_failure = 0;
    simulated_calib_trco_status = 0x80;
    simulated_calib_srco_status = 0x80;
    simulate_write_0_failure = 0;
    simulate_write_1_failure = 0;
    simulate_write_2_failure = 0;
    simulate_write_3_failure = 0;
}

void tearDown(void) {
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
    
    state.log_file = tmpfile(); //archivo temporal para simular el log
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

void test_as3935_init_fails_on_preset_default_write(void) {

    simulate_write_preset_failure = 1;
    struct SystemState state;
    memset(&state, 0, sizeof(state));

    as3935_fopen_IgnoreAndReturn(tmpfile());
    log_timestamp_Ignore();

    int result = as3935_init(&state);

    TEST_ASSERT_EQUAL(-1, result);
}

void test_as3935_init_fails_on_calib_rco_write(void) {
    
    simulate_write_calib_failure = 1;
    struct SystemState state;
    memset(&state, 0, sizeof(state));

    as3935_fopen_IgnoreAndReturn(tmpfile());
    log_timestamp_Ignore();

    int result = as3935_init(&state);

    TEST_ASSERT_EQUAL(-1, result);
}

void test_as3935_init_fails_on_3a_read(void) {
    struct SystemState state;
    memset(&state, 0, sizeof(state));

    simulate_read_3a_failure = 1;

    as3935_fopen_IgnoreAndReturn(tmpfile());
    log_timestamp_Ignore();

    int result = as3935_init(&state);

    TEST_ASSERT_EQUAL(-1, result);
}

void test_as3935_init_fails_on_3b_read(void) {
    struct SystemState state;
    memset(&state, 0, sizeof(state));

    simulate_read_3b_failure = 1;

    as3935_fopen_IgnoreAndReturn(tmpfile());
    log_timestamp_Ignore();

    int result = as3935_init(&state);

    TEST_ASSERT_EQUAL(-1, result);
}

void test_as3935_init_fails_trco_check(void) {
    struct SystemState state;
    memset(&state, 0, sizeof(state));

    simulated_calib_trco_status = 0x00;

    as3935_fopen_IgnoreAndReturn(tmpfile());
    log_timestamp_Ignore();

    int result = as3935_init(&state);

    TEST_ASSERT_EQUAL(-1, result);
}

void test_as3935_init_fails_srco_check(void) {
    struct SystemState state;
    memset(&state, 0, sizeof(state));

    simulated_calib_srco_status = 0x00;

    as3935_fopen_IgnoreAndReturn(tmpfile());
    log_timestamp_Ignore();

    int result = as3935_init(&state);

    TEST_ASSERT_EQUAL(-1, result);
}

void test_as3935_init_fails_on_reg_0_write(void) {
    struct SystemState state;
    memset(&state, 0, sizeof(state));

    simulate_write_0_failure = 1;

    as3935_fopen_IgnoreAndReturn(tmpfile());
    log_timestamp_Ignore();

    int result = as3935_init(&state);

    TEST_ASSERT_EQUAL(-1, result);
}

void test_as3935_init_fails_on_reg_1_write(void) {
    struct SystemState state;
    memset(&state, 0, sizeof(state));

    simulate_write_1_failure = 1;

    as3935_fopen_IgnoreAndReturn(tmpfile());
    log_timestamp_Ignore();

    int result = as3935_init(&state);

    TEST_ASSERT_EQUAL(-1, result);
}

void test_as3935_init_fails_on_reg_2_write(void) {
    struct SystemState state;
    memset(&state, 0, sizeof(state));

    simulate_write_2_failure = 1;

    as3935_fopen_IgnoreAndReturn(tmpfile());
    log_timestamp_Ignore();

    int result = as3935_init(&state);

    TEST_ASSERT_EQUAL(-1, result);
}

void test_as3935_init_fails_on_3_read(void) {
    struct SystemState state;
    memset(&state, 0, sizeof(state));

    simulate_read_3_failure = 1;

    as3935_fopen_IgnoreAndReturn(tmpfile());
    log_timestamp_Ignore();

    int result = as3935_init(&state);

    TEST_ASSERT_EQUAL(-1, result);
}

void test_as3935_init_fails_on_reg_3_write(void) {
    struct SystemState state;
    memset(&state, 0, sizeof(state));

    simulate_write_3_failure = 1;

    as3935_fopen_IgnoreAndReturn(tmpfile());
    log_timestamp_Ignore();

    int result = as3935_init(&state);

    TEST_ASSERT_EQUAL(-1, result);
}