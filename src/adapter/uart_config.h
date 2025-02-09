typedef struct
{
    bool four_wire;
    int baud_rate;
    int stop_bits;
    String termination_str;
} uartConfig_t;

const uartConfig_t AGILENT_34401A = {true, 9600, 2, "\n"};
const uartConfig_t TENMA_PSU = {false, 9600, 1, ""};
const uartConfig_t AGILENT_E4433B = {false, 9600, 1, "\r\n"};