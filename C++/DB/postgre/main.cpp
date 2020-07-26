/*
 * src/test/examples/testlibpq.c
 *
 *
 * testlibpq.c
 *
 *      Test the C version of libpq, the PostgreSQL frontend library.
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <postgresql/libpq-fe.h>

static void
exit_nicely(PGconn *conn)
{
    PQfinish(conn);
    exit(1);
}

int defaultExample(int argc, char **argv);
void showUser();

int main(int argc, char **argv)
{
    //defaultExample(argc,argv);
    showUser();
}

int defaultExample(int argc, char **argv)
{
    const char *conninfo;
    PGconn *conn;
    PGresult *res;
    int nFields;
    int i,
        j;

    /*
     * If the user supplies a parameter on the command line, use it as the
     * conninfo string; otherwise default to setting dbname=postgres and using
     * environment variables or defaults for all other connection parameters.
     */
    // dbname = rate-races user = postgres password = pass hostaddr = 0.0.0.0
    if (argc > 1)
        conninfo = argv[1];
    else
        conninfo = "dbname = rate-races user = postgres password = pass hostaddr = 0.0.0.0";

    /* Make a connection to the database */
    conn = PQconnectdb(conninfo);

    /* Check to see that the backend connection was successfully made */
    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection to database failed: %s",
                PQerrorMessage(conn));
        exit_nicely(conn);
    }

    /* Set always-secure search path, so malicious users can't take control. */
    res = PQexec(conn,
                 "SELECT pg_catalog.set_config('search_path', '', false)");
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "SET failed: %s", PQerrorMessage(conn));
        PQclear(res);
        exit_nicely(conn);
    }

    /*
     * Should PQclear PGresult whenever it is no longer needed to avoid memory
     * leaks
     */
    PQclear(res);

    /*
     * Our test case here involves using a cursor, for which we must be inside
     * a transaction block.  We could do the whole thing with a single
     * PQexec() of "select * from pg_database", but that's too trivial to make
     * a good example.
     */

    /* Start a transaction block */
    res = PQexec(conn, "BEGIN");
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(conn));
        PQclear(res);
        exit_nicely(conn);
    }
    PQclear(res);

    /*
     * Fetch rows from pg_database, the system catalog of databases
     */
    res = PQexec(conn, "DECLARE myportal CURSOR FOR select * from pg_database");
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "DECLARE CURSOR failed: %s", PQerrorMessage(conn));
        PQclear(res);
        exit_nicely(conn);
    }
    PQclear(res);

    res = PQexec(conn, "FETCH ALL in myportal");
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "FETCH ALL failed: %s", PQerrorMessage(conn));
        PQclear(res);
        exit_nicely(conn);
    }

    /* first, print out the attribute names */
    nFields = PQnfields(res);
    for (i = 0; i < nFields; i++)
        printf("%-15s", PQfname(res, i));
    printf("\n\n");

    /* next, print out the rows */
    for (i = 0; i < PQntuples(res); i++)
    {
        for (j = 0; j < nFields; j++)
            printf("%-15s", PQgetvalue(res, i, j));
        printf("\n");
    }

    PQclear(res);

    /* close the portal ... we don't bother to check for errors ... */
    res = PQexec(conn, "CLOSE myportal");
    PQclear(res);

    /* end the transaction */
    res = PQexec(conn, "END");
    PQclear(res);

    /* close the connection to the database and cleanup */
    PQfinish(conn);

    return 0;
}

void showUser()
{

    const char *conninfo;
    PGconn *conn;
    PGresult *res;

    const char *connInfoAux = "postgresql://postgres@localhost?port=5432&dbname=rate-races&user=postgres&password=pass";
    conninfo = "dbname = rate-races user = postgres password = pass hostaddr = 0.0.0.0";

    conn = PQconnectdb(connInfoAux);

    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection to database failed: %s",
                PQerrorMessage(conn));
        exit_nicely(conn);
    }

    res = PQexec(conn, "select name,id from race;");


    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        std::cout << "Select failed: " << PQresultErrorMessage(res) << std::endl;
        PQclear(res);
        exit_nicely(conn);
    }

    int nTuples = PQntuples(res);
    int nFields = PQnfields(res);
    printf("%d %d \n", nTuples, nFields);

    int i, j;

    /* for (i = 0; i < nFields; ++i)
    {
        printf("%-15s", PQfname(res, i));
    } */

    for (int i = 0; i < PQntuples(res); i++) {
      for (int j = 0; j < PQnfields(res); j++) {
        std::cout << PQgetvalue(res, i, j) << "   ";
      }
      std::cout << std::endl;
    }

    PQclear(res);

    PQfinish(conn);
}
