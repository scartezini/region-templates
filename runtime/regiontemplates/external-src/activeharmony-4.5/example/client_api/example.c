/*
 * Copyright 2003-2013 Jeffrey K. Hollingsworth
 *
 * This file is part of Active Harmony.
 *
 * Active Harmony is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Active Harmony is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Active Harmony.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>

#include "hclient.h"
#include "defaults.h"

#define MAX_LOOP 5000

/* For illustration purposes, the performance here is defined by following
 * simple definition:
 *   perf = (p1 - 15)^2 + (p2 - 30)^2 + (p3 - 45)^2 +
 *          (p4 - 60)^2 + (p5 - 75)^2 + (p6 - 90)^2
 *
 * So the theoretical minimum can be found at point:
 *      (15, 30, 45, 60, 75, 90)
 *
 * And a reasonable search range for all parameters is [1-100].
 *
 */
long application(long p1, long p2, long p3, long p4, long p5, long p6)
{
    long perf =
        (p1-15) * (p1-15) +
        (p2-30) * (p2-30) +
        (p3-45) * (p3-45) +
        (p4-60) * (p4-60) +
        (p5-75) * (p5-75) +
        (p6-90) * (p6-90);
    return perf;
}

int main(int argc, char **argv)
{
    char name[1024];
    hdesc_t *hdesc, *hdesc1;
    int i, retval;
    double perf;

    /* Variables to hold the application's runtime tunable parameters.
     * Once bound to a Harmony tuning session, these variables will be
     * modified upon harmony_fetch() to a new testing point.
     */
    long param_1;
    long param_2;
    long param_3;
    long param_4;
    long param_5;
    long param_6;

    long param_1_1;
    long param_2_1;
    long param_3_1;
    long param_4_1;
    long param_5_1;
    long param_6_1;

    retval = 0;
    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            fprintf(stderr, "Usage: %s [session_name]"
                    " [KEY_1=VAL_1] .. [KEY_N=VAL_N]\n\n", argv[0]);
            return 0;
        }
    }

    /* Initialize a Harmony client. */
    hdesc = harmony_init(&argc, &argv);
   hdesc1 = harmony_init(&argc, &argv);
    if (hdesc == NULL) {
        fprintf(stderr, "Failed to initialize a harmony session.\n");
        return -1;
    }

    /* Process the program arguments. */
    if (argc > 1 && !strchr(argv[1], '=')) {
        strncpy(name, argv[1], sizeof(name));
    }
    else {
        snprintf(name, sizeof(name), "c_example.%d", getpid());
    }

    if (harmony_session_name(hdesc, name) != 0) {
        fprintf(stderr, "Could not set session name.\n");
        return -1;
    }

    if (harmony_int(hdesc, "param_1", 1, 100, 1) != 0 ||
        harmony_int(hdesc, "param_2", 1, 100, 1) != 0 ||
        harmony_int(hdesc, "param_3", 1, 100, 1) != 0 ||
        harmony_int(hdesc, "param_4", 1, 100, 1) != 0 ||
        harmony_int(hdesc, "param_5", 1, 100, 1) != 0 ||
        harmony_int(hdesc, "param_6", 1, 100, 1) != 0)
    {
        fprintf(stderr, "Failed to define tuning session\n");
        return -1;
    }


 	harmony_strategy(hdesc, "pro.so");
//	harmony_setcfg(hdesc, "PER_CLIENT_STORAGE", "2");
        harmony_setcfg(hdesc, CFGKEY_CLIENT_COUNT, "2");
        harmony_setcfg(hdesc, CFGKEY_SIMPLEX_SIZE, "7");

    printf("Starting Harmony...\n");
    if (harmony_launch(hdesc, NULL, 0) != 0) {
        fprintf(stderr, "Could not launch tuning session: %s\n",
                harmony_error_string(hdesc));
        return -1;
    }

    printf("After Starting Harmony...\n");
    /* Bind the session variables to local variables. */
    if (harmony_bind_int(hdesc, "param_1", &param_1) != 0 ||
        harmony_bind_int(hdesc, "param_2", &param_2) != 0 ||
        harmony_bind_int(hdesc, "param_3", &param_3) != 0 ||
        harmony_bind_int(hdesc, "param_4", &param_4) != 0 ||
        harmony_bind_int(hdesc, "param_5", &param_5) != 0 ||
        harmony_bind_int(hdesc, "param_6", &param_6) != 0)
    {
        fprintf(stderr, "Failed to register variable\n");
        retval = -1;
        goto cleanup;
    }


    printf("After bind 1...\n");

    /* Bind the session variables to local variables. */
    if (harmony_bind_int(hdesc1, "param_1", &param_1_1) != 0 ||
        harmony_bind_int(hdesc1, "param_2", &param_2_1) != 0 ||
        harmony_bind_int(hdesc1, "param_3", &param_3_1) != 0 ||
        harmony_bind_int(hdesc1, "param_4", &param_4_1) != 0 ||
        harmony_bind_int(hdesc1, "param_5", &param_5_1) != 0 ||
        harmony_bind_int(hdesc1, "param_6", &param_6_1) != 0)
    {
        fprintf(stderr, "Failed to register variable\n");
        retval = -1;
        goto cleanup;
    }

    printf("After bind 2...\n");
    /* Join this client to the tuning session we established above. */
    if (harmony_join(hdesc, NULL, 0, name) != 0) {
        fprintf(stderr, "Could not connect to harmony server: %s\n",
                harmony_error_string(hdesc));
        retval = -1;
        goto cleanup;
    }

    printf("After join 1...\n");
    /* Join this client to the tuning session we established above. */
    if (harmony_join(hdesc1, NULL, 0, name) != 0) {
        fprintf(stderr, "Could not connect to harmony server: %s\n",
                harmony_error_string(hdesc1));
        retval = -1;
        goto cleanup;
    }

    printf("After join 2...\n");
    /* main loop */
    for (i = 0; !harmony_converged(hdesc) && i < MAX_LOOP; ++i) {
        int hresult = harmony_fetch(hdesc);
        int hresult1 = harmony_fetch(hdesc1);
        if (hresult < 0) {
            fprintf(stderr, "Failed to fetch values from server: %s\n",
                    harmony_error_string(hdesc));
            retval = -1;
            goto cleanup;
        }
        else if (hresult == 0) {
            /* New values were not available at this time.
             * Bundles remain unchanged by Harmony system.
             */
        }
        else if (hresult > 0) {
            /* The Harmony system modified the variable values.
             * Do any systemic updates to deal with such a change.
             */
        }


        /* Run one full iteration of the application (or code variant).
         *
         * Here our application is rather simple. Definition of performance can
         * be user-defined. Depending on application, it can be MFlops/sec,
         * time to complete the entire run of the application, cache hits vs.
         * misses and so on.
         *
         * For searching the parameter space in a Transformation framework,
         * just run different parameterized code variants here. A simple
         * mapping between the parameters and the code-variants is needed to
         * call the appropriate code variant.
         */

	perf = application(param_1, param_2, param_3,
			param_4, param_5, param_6);

	if (hresult > 0) {
		/* Only print performance if new values were fetched. */
		printf("%3ld, %3ld, %3ld, %3ld, %3ld, %3ld = %lf\n",
				param_1, param_2, param_3, param_4, param_5, param_6, perf);
	}

	/* Report the performance we've just measured. */
	if (harmony_report(hdesc, perf) != 0) {
		fprintf(stderr, "Failed to report performance to server: .\n");
		retval = -1;
		goto cleanup;
	}

	perf = application(param_1_1, param_2_1, param_3_1,
			param_4_1, param_5_1, param_6_1);

	if (hresult1 > 0) {
		/* Only print performance if new values were fetched. */
		printf("%3ld, %3ld, %3ld, %3ld, %3ld, %3ld = %lf\n",
				param_1_1, param_2_1, param_3_1, param_4_1, param_5_1, param_6_1, perf);
	}

	/* Report the performance we've just measured. */
	if (harmony_report(hdesc1, perf) != 0) {
		fprintf(stderr, "Failed to report performance to server: .\n");
		retval = -1;
		goto cleanup;
	}

    }

    if (!harmony_converged(hdesc)) {
        printf("*\n");
        printf("* Leaving tuning session after %d iterations.\n", MAX_LOOP);
        printf("*\n");
    }

    /* Leave the session */
    if (harmony_leave(hdesc) != 0) {
        fprintf(stderr, "Failed to disconnect from harmony server.\n");
        retval = -1;
    }

    if (harmony_best(hdesc) == 0) {
        printf("\n");
        printf("Optimal point:      ( 15,  30,  45,  60,  75,  90)\n");
        printf("Best point found:   "
               "(%3ld, %3ld, %3ld, %3ld, %3ld, %3ld)\n",
               param_1, param_2, param_3, param_4, param_5, param_6);

        perf = ((15 - param_1) * (15 - param_1) +
                (30 - param_2) * (30 - param_2) +
                (45 - param_3) * (45 - param_3) +
                (60 - param_4) * (60 - param_4) +
                (75 - param_5) * (75 - param_5) +
                (90 - param_6) * (90 - param_6));
        printf("Euclidean distance: %lf\n", sqrt(perf));
    }
    else {
        printf("\nBest point could not be retrieved.\n");
    }


  cleanup:
    harmony_fini(hdesc);
    return retval;
}
