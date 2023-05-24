#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <assert.h>

#include "../src/cat.h"

static uint16_t bytesToSend;
static uint16_t bytesRead = 0;
static uint8_t sendBuffer[64] = { 0 };

static struct cat_variable cipsend_vars[] = {
        {
                .type = CAT_VAR_INT_DEC,
                .data = &bytesToSend,
                .data_size = sizeof(bytesToSend),
                .name = "bytesToSend",
                .access = CAT_VAR_ACCESS_READ_WRITE,
        },
};

/* helper variable used to exit demo code */
static bool quit_flag;

static char connection_type[4];
static char connection_ip[46];
static uint16_t connection_port;

static uint16_t recv_data_length;

/* custom target dependent input output handlers */
static int write_char(char ch)
{
        putc(ch, stdout);
        return 1;
}

static int read_char(char *ch)
{
        *ch = getc(stdin);
        return 1;
}

static int cgmm_run(const struct cat_command *cmd)
{
    //TODO: Also print version?
    printf("CC1352P7\n");
    return CAT_RETURN_STATE_DATA_OK;
}

static struct cat_variable cipstart_vars[] = {
        {
                .type = CAT_VAR_BUF_STRING,
                .data = connection_type,
                .data_size = sizeof(connection_type),
                .name = "type"
        },
        {
                .type = CAT_VAR_BUF_STRING,
                .data = connection_ip,
                .data_size = sizeof(connection_ip),
                .name = "remote IP"
        },
        {
                .type = CAT_VAR_INT_DEC,
                .data = &connection_port,
                .data_size = sizeof(connection_port),
                .name = "remote port"
        }
};

static int cipstart_run(const struct cat_command *cmd, const uint8_t *data, const size_t data_size, const size_t args_num)
{
    printf("Type: %s, IP: %s, port: %d\n", connection_type, connection_ip, connection_port);
    return CAT_RETURN_STATE_DATA_OK;
}

static int cipclose_run(const struct cat_command *cmd)
{
    return CAT_RETURN_STATE_ERROR;
}

static int cipsend_read(const struct cat_command *cmd, const uint8_t *data, const size_t data_size, const size_t args_num)
{
        if(bytesRead==0){
                write_char('>');
        }
        uint16_t min_size = bytesToSend > sizeof(sendBuffer) ? sizeof(sendBuffer) : bytesToSend;
        for (uint16_t i = 0; i < min_size; i++){
                read_char(&sendBuffer[i]);
                printf("%c", sendBuffer[i]);
        }
        bytesToSend -= min_size;
        if(bytesToSend > 0){
                bytesRead += min_size;
                return CAT_RETURN_STATE_DATA_NEXT;
        }
        bytesRead = 0;
        return CAT_RETURN_STATE_DATA_OK;
}

static struct cat_variable ciprecvdata_vars[] = {
        {
                .type = CAT_VAR_INT_DEC,
                .data = &recv_data_length,
                .data_size = sizeof(recv_data_length),
                .name = "data length"
        }
};

static int ciprecvdata_run(const struct cat_command *cmd, const uint8_t *data, const size_t data_size, const size_t args_num)
{
    return CAT_RETURN_STATE_DATA_OK;;
}

/* declaring commands array */
static struct cat_command cmds[] = {
        {
                .name = "+CGMM",
                .description = "Modem model",
                .run = cgmm_run
        },
        {
                .name = "+CIPSTART",
                .description = "Connect TCP",
                .write = cipstart_run,
                .var = cipstart_vars,
                .var_num = sizeof(cipstart_vars) / sizeof(cipstart_vars[0]),
                .need_all_vars = true
        },
        {
                .name = "+CIPCLOSE",
                .description = "Close connection",
                .run = cipclose_run
        },
        {
                .name = "+CIPSEND",
                .description = "Send data to peer",
                .write = cipsend_read,
                .var = cipsend_vars,
                .var_num = sizeof(cipsend_vars) / sizeof(cipsend_vars[0]),
        },
        {
                .name = "+CIPRECVDATA",
                .description = "Receive data from peer",
                .write = ciprecvdata_run,
                .var = ciprecvdata_vars,
                .var_num = sizeof(ciprecvdata_vars) / sizeof(ciprecvdata_vars[0]),
                .need_all_vars = true
        },
};

/* working buffer */
static char buf[128];

/* declaring parser descriptor */
static struct cat_command_group cmd_group = {
        .cmd = cmds,
        .cmd_num = sizeof(cmds) / sizeof(cmds[0]),
};

static struct cat_command_group *cmd_desc[] = {
        &cmd_group
};

static struct cat_descriptor desc = {
        .cmd_group = cmd_desc,
        .cmd_group_num = sizeof(cmd_desc) / sizeof(cmd_desc[0]),

        .buf = buf,
        .buf_size = sizeof(buf)
};

/* declaring input output interface descriptor for parser */
static struct cat_io_interface iface = {
        .read = read_char,
        .write = write_char
};

int main(int argc, char **argv)
{
        struct cat_object at;

        /* initializing */
        cat_init(&at, &desc, &iface, NULL);

        /* main loop with exit code conditions */
        while ((cat_service(&at) != 0) && (quit_flag == 0)) {};

        /* goodbye message */
        printf("Bye!\n");

        return 0;
}
