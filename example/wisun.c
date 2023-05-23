#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <assert.h>

#include "../src/cat.h"

/* helper variable used to exit demo code */
static bool quit_flag;

static char connection_type[4];
static char connection_ip[46];
static uint16_t connection_port;

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

static int cipsend_run(const struct cat_command *cmd)
{
    return 0;
}

static int ciprecvdata_run(const struct cat_command *cmd)
{
    return 0;
}

/* declaring commands array */
static struct cat_command cmds[] = {
        {
                .name = "+CGMM",
                .description = "Modem model",
                .run = cgmm_run,
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
                .run = cipclose_run,
        },
        {
                .name = "+CIPSEND",
                .description = "Send data to peer",
                .run = cipsend_run,
        },
        {
                .name = "+CIPRECVDATA",
                .description = "Receive data from peer",
                .run = ciprecvdata_run,
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
