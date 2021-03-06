/*
 * src/bin/pg_autoctl/cli_do_root.c
 *     Implementation of a CLI which lets you run operations on the local
 *     postgres server directly.
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the PostgreSQL License.
 *
 */

#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <signal.h>

#include "postgres_fe.h"

#include "cli_common.h"
#include "cli_do_root.h"
#include "commandline.h"
#include "config.h"
#include "defaults.h"
#include "file_utils.h"
#include "fsm.h"
#include "keeper_config.h"
#include "keeper.h"
#include "monitor.h"
#include "monitor_config.h"
#include "pgctl.h"
#include "primary_standby.h"


CommandLine do_primary_adduser_replica =
	make_command("replica",
				 "add a local user with replication privileges",
				 "",
				 KEEPER_CLI_WORKER_SETUP_OPTIONS,
				 keeper_cli_keeper_setup_getopts,
				 keeper_cli_create_replication_user);

CommandLine *do_primary_adduser_subcommands[] = {
	&do_primary_adduser_replica,
	NULL
};

CommandLine do_primary_adduser =
	make_command_set("adduser",
					 "Create users on primary", NULL, NULL,
					 NULL, do_primary_adduser_subcommands);

CommandLine do_primary_syncrep_enable =
	make_command("enable",
				 "Enable synchronous replication on the primary server",
				 "",
				 "",
				 NULL, keeper_cli_enable_synchronous_replication);

CommandLine do_primary_syncrep_disable =
	make_command("disable",
				 "Disable synchronous replication on the primary server",
				 "",
				 "",
				 NULL, keeper_cli_disable_synchronous_replication);

CommandLine *do_primary_syncrep[] = {
	&do_primary_syncrep_enable,
	&do_primary_syncrep_disable,
	NULL
};

CommandLine do_primary_syncrep_ =
	make_command_set("syncrep",
					 "Manage the synchronous replication setting on the primary server",
					 NULL, NULL,
					 NULL, do_primary_syncrep);

CommandLine do_primary_slot_create =
	make_command("create",
				 "Create a replication slot on the primary server",
				 "",
				 KEEPER_CLI_WORKER_SETUP_OPTIONS,
				 keeper_cli_keeper_setup_getopts,
				 keeper_cli_create_replication_slot);

CommandLine do_primary_slot_drop =
	make_command("drop",
				 "Drop a replication slot on the primary server",
				 "",
				 KEEPER_CLI_WORKER_SETUP_OPTIONS,
				 keeper_cli_keeper_setup_getopts,
				 keeper_cli_drop_replication_slot);

CommandLine *do_primary_slot[] = {
	&do_primary_slot_create,
	&do_primary_slot_drop,
	NULL
};

CommandLine do_primary_slot_ =
	make_command_set("slot",
					 "Manage replication slot on the primary server", NULL, NULL,
					 NULL, do_primary_slot);

CommandLine do_primary_defaults =
	make_command("defaults",
				 "Add default settings to postgresql.conf",
				 "",
				 KEEPER_CLI_WORKER_SETUP_OPTIONS,
				 keeper_cli_keeper_setup_getopts,
				 keeper_cli_add_default_settings);

CommandLine do_primary_hba_setup =
	make_command("setup",
				 "Make sure the standby has replication access in pg_hba",
				 "<standby hostname>",
				 KEEPER_CLI_WORKER_SETUP_OPTIONS,
				 keeper_cli_keeper_setup_getopts,
				 keeper_cli_add_standby_to_hba);

CommandLine *do_primary_hba_commands[] = {
	&do_primary_hba_setup,
	NULL
};

CommandLine do_primary_hba =
	make_command_set("hba",
					 "Manage pg_hba settings on the primary server", NULL, NULL,
					 NULL, do_primary_hba_commands);


CommandLine *do_primary[] = {
	&do_primary_slot_,
	&do_primary_syncrep_,
	&do_primary_defaults,
	&do_primary_adduser,
	&do_primary_hba,
	NULL
};

CommandLine do_primary_ =
	make_command_set("primary",
					 "Manage a PostgreSQL primary server", NULL, NULL,
					 NULL, do_primary);

CommandLine do_standby_init =
	make_command("init",
				 "Initialize the standby server using pg_basebackup",
				 "[option ...] <primary name> <primary port> \n",
				 KEEPER_CLI_WORKER_SETUP_OPTIONS,
				 keeper_cli_keeper_setup_getopts,
				 keeper_cli_init_standby);

CommandLine do_standby_rewind =
	make_command("rewind",
				 "Rewind a demoted primary server using pg_rewind",
				 "<primary host> <primary port>",
				 KEEPER_CLI_WORKER_SETUP_OPTIONS,
				 keeper_cli_keeper_setup_getopts,
				 keeper_cli_rewind_old_primary);

CommandLine do_standby_promote =
	make_command("promote",
				 "Promote a standby server to become writable",
				 "",
				 KEEPER_CLI_WORKER_SETUP_OPTIONS,
				 keeper_cli_keeper_setup_getopts,
				 keeper_cli_promote_standby);

CommandLine *do_standby[] = {
	&do_standby_init,
	&do_standby_rewind,
	&do_standby_promote,
	NULL
};

CommandLine do_standby_ =
	make_command_set("standby",
					 "Manage a PostgreSQL standby server", NULL, NULL,
					 NULL, do_standby);

CommandLine do_pgsetup_discover =
	make_command("discover",
				 "Discover local PostgreSQL instance, if any",
				 "[option ...]",
				 KEEPER_CLI_WORKER_SETUP_OPTIONS,
				 keeper_cli_keeper_setup_getopts,
				 keeper_cli_pgsetup_discover);

CommandLine do_pgsetup_is_ready =
	make_command("ready",
				 "Return true is the local Postgres server is ready",
				 "[option ...]",
				 KEEPER_CLI_WORKER_SETUP_OPTIONS,
				 keeper_cli_keeper_setup_getopts,
				 keeper_cli_pgsetup_is_ready);

CommandLine do_pgsetup_wait_until_ready =
	make_command("wait",
				 "Wait until the local Postgres server is ready",
				 "[option ...]",
				 KEEPER_CLI_WORKER_SETUP_OPTIONS,
				 keeper_cli_keeper_setup_getopts,
				 keeper_cli_pgsetup_wait_until_ready);

CommandLine do_pgsetup_startup_logs =
	make_command("logs",
				 "Outputs the Postgres startup logs",
				 "[option ...]",
				 KEEPER_CLI_WORKER_SETUP_OPTIONS,
				 keeper_cli_keeper_setup_getopts,
				 keeper_cli_pgsetup_startup_logs);

CommandLine *do_pgsetup[] = {
	&do_pgsetup_discover,
	&do_pgsetup_is_ready,
	&do_pgsetup_wait_until_ready,
	&do_pgsetup_startup_logs,
	NULL
};

CommandLine do_pgsetup_commands =
	make_command_set("pgsetup",
					 "Manage a local Postgres setup", NULL, NULL,
					 NULL, do_pgsetup);

CommandLine *do_subcommands[] = {
	&do_monitor_commands,
	&do_fsm_commands,
	&do_primary_,
	&do_standby_,
	&do_show_commands,
	&do_pgsetup_commands,
	&do_service_postgres_ctl_commands,
	&do_service_commands,
	NULL
};

CommandLine do_commands =
	make_command_set("do",
					 "Manually operate the keeper", NULL, NULL,
					 NULL, do_subcommands);


/*
 * keeper_cli_keeper_setup_getopts parses command line options and set the
 * global variable keeperOptions from them, without doing any check.
 */
int
keeper_cli_keeper_setup_getopts(int argc, char **argv)
{
	KeeperConfig options = { 0 };
	int optind;

	SSLCommandLineOptions sslCommandLineOptions = SSL_CLI_UNKNOWN;

	static struct option long_options[] = {
		{ "pgctl", required_argument, NULL, 'C' },
		{ "pgdata", required_argument, NULL, 'D' },
		{ "pghost", required_argument, NULL, 'H' },
		{ "pgport", required_argument, NULL, 'p' },
		{ "listen", required_argument, NULL, 'l' },
		{ "username", required_argument, NULL, 'U' },
		{ "auth", required_argument, NULL, 'A' },
		{ "skip-pg-hba", no_argument, NULL, 'S' },
		{ "dbname", required_argument, NULL, 'd' },
		{ "nodename", required_argument, NULL, 'n' },
		{ "formation", required_argument, NULL, 'f' },
		{ "monitor", required_argument, NULL, 'm' },
		{ "disable-monitor", no_argument, NULL, 'M' },
		{ "version", no_argument, NULL, 'V' },
		{ "verbose", no_argument, NULL, 'v' },
		{ "quiet", no_argument, NULL, 'q' },
		{ "help", no_argument, NULL, 'h' },
		{ "candidate-priority", required_argument, NULL, 'P' },
		{ "replication-quorum", required_argument, NULL, 'r' },
		{ "run", no_argument, NULL, 'x' },
		{ "help", no_argument, NULL, 0 },
		{ "no-ssl", no_argument, NULL, 'N' },
		{ "ssl-self-signed", no_argument, NULL, 's' },
		{ "ssl-mode", required_argument, &ssl_flag, SSL_MODE_FLAG },
		{ "ssl-ca-file", required_argument, &ssl_flag, SSL_CA_FILE_FLAG },
		{ "ssl-crl-file", required_argument, &ssl_flag, SSL_CRL_FILE_FLAG },
		{ "server-cert", required_argument, &ssl_flag, SSL_SERVER_CRT_FLAG },
		{ "server-key", required_argument, &ssl_flag, SSL_SERVER_KEY_FLAG },
		{ NULL, 0, NULL, 0 }
	};

	/*
	 * The only command lines that are using keeper_cli_getopt_pgdata are
	 * terminal ones: they don't accept subcommands. In that case our option
	 * parsing can happen in any order and we don't need getopt_long to behave
	 * in a POSIXLY_CORRECT way.
	 *
	 * The unsetenv() call allows getopt_long() to reorder arguments for us.
	 */
	unsetenv("POSIXLY_CORRECT");

	optind = cli_common_keeper_getopts(argc, argv,
									   long_options,
									   "C:D:H:p:l:U:A:Sd:n:f:m:MRVvqhP:r:xsN",
									   &options,
									   &sslCommandLineOptions);

	/* publish our option parsing in the global variable */
	keeperOptions = options;

	return optind;
}
