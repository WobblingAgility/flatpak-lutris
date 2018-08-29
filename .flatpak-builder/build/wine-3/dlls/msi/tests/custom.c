/*
 * DLL for testing type 1 custom actions
 *
 * Copyright 2017 Zebediah Figura
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdarg.h>
#include <stdio.h>

#include <windef.h>
#include <winbase.h>
#define COBJMACROS
#include <msxml.h>
#include <msi.h>
#include <msiquery.h>

static void ok_(MSIHANDLE hinst, int todo, const char *file, int line, int condition, const char *msg, ...)
{
    static char buffer[2000];
    MSIHANDLE record;
    va_list valist;

    va_start(valist, msg);
    vsprintf(buffer, msg, valist);
    va_end(valist);

    record = MsiCreateRecord(5);
    MsiRecordSetInteger(record, 1, todo);
    MsiRecordSetStringA(record, 2, file);
    MsiRecordSetInteger(record, 3, line);
    MsiRecordSetInteger(record, 4, condition);
    MsiRecordSetStringA(record, 5, buffer);
    MsiProcessMessage(hinst, INSTALLMESSAGE_USER, record);
    MsiCloseHandle(record);
}
#define ok(hinst, condition, ...)           ok_(hinst, 0, __FILE__, __LINE__, condition, __VA_ARGS__)
#define todo_wine_ok(hinst, condition, ...) ok_(hinst, 1, __FILE__, __LINE__, condition, __VA_ARGS__)

static const char *dbgstr_w(WCHAR *str)
{
    static char buffer[300], *p;

    if (!str) return "(null)";

    p = buffer;
    *p++ = 'L';
    *p++ = '"';
    while ((*p++ = *str++));
    *p++ = '"';
    *p++ = 0;

    return buffer;
}

static void check_prop(MSIHANDLE hinst, const char *prop, const char *expect)
{
    char buffer[10] = "x";
    DWORD sz = sizeof(buffer);
    UINT r = MsiGetPropertyA(hinst, prop, buffer, &sz);
    ok(hinst, !r, "'%s': got %u\n", prop, r);
    ok(hinst, sz == strlen(buffer), "'%s': expected %u, got %u\n", prop, strlen(buffer), sz);
    ok(hinst, !strcmp(buffer, expect), "expected '%s', got '%s'\n", expect, buffer);
}

static void test_props(MSIHANDLE hinst)
{
    static const WCHAR booW[] = {'b','o','o',0};
    static const WCHAR xyzW[] = {'x','y','z',0};
    static const WCHAR xyW[] = {'x','y',0};
    char buffer[10];
    WCHAR bufferW[10];
    DWORD sz;
    UINT r;

    /* test invalid values */
    r = MsiGetPropertyA(hinst, NULL, NULL, NULL);
    ok(hinst, r == ERROR_INVALID_PARAMETER, "got %u\n", r);

    r = MsiGetPropertyA(hinst, "boo", NULL, NULL);
    ok(hinst, !r, "got %u\n", r);

    r = MsiGetPropertyA(hinst, "boo", buffer, NULL );
    ok(hinst, r == ERROR_INVALID_PARAMETER, "got %u\n", r);

    sz = 0;
    r = MsiGetPropertyA(hinst, "boo", NULL, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, sz == 0, "got size %u\n", sz);

    sz = 0;
    strcpy(buffer,"x");
    r = MsiGetPropertyA(hinst, "boo", buffer, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !strcmp(buffer, "x"), "got \"%s\"\n", buffer);
    ok(hinst, sz == 0, "got size %u\n", sz);

    sz = 1;
    strcpy(buffer,"x");
    r = MsiGetPropertyA(hinst, "boo", buffer, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, !buffer[0], "got \"%s\"\n", buffer);
    ok(hinst, sz == 0, "got size %u\n", sz);

    /* set the property to something */
    r = MsiSetPropertyA(hinst, NULL, NULL);
    ok(hinst, r == ERROR_INVALID_PARAMETER, "got %u\n", r);

    r = MsiSetPropertyA(hinst, "", NULL);
    ok(hinst, !r, "got %u\n", r);

    r = MsiSetPropertyA(hinst, "", "asdf");
    ok(hinst, r == ERROR_FUNCTION_FAILED, "got %u\n", r);

    r = MsiSetPropertyA(hinst, "=", "asdf");
    ok(hinst, !r, "got %u\n", r);
    check_prop(hinst, "=", "asdf");

    r = MsiSetPropertyA(hinst, " ", "asdf");
    ok(hinst, !r, "got %u\n", r);
    check_prop(hinst, " ", "asdf");

    r = MsiSetPropertyA(hinst, "'", "asdf");
    ok(hinst, !r, "got %u\n", r);
    check_prop(hinst, "'", "asdf");

    r = MsiSetPropertyA(hinst, "boo", NULL);
    ok(hinst, !r, "got %u\n", r);
    check_prop(hinst, "boo", "");

    r = MsiSetPropertyA(hinst, "boo", "");
    ok(hinst, !r, "got %u\n", r);
    check_prop(hinst, "boo", "");

    r = MsiSetPropertyA(hinst, "boo", "xyz");
    ok(hinst, !r, "got %u\n", r);
    check_prop(hinst, "boo", "xyz");

    r = MsiGetPropertyA(hinst, "boo", NULL, NULL);
    ok(hinst, !r, "got %u\n", r);

    r = MsiGetPropertyA(hinst, "boo", buffer, NULL );
    ok(hinst, r == ERROR_INVALID_PARAMETER, "got %u\n", r);

    /* Returned size is in bytes, not chars, but only for custom actions.
     * Seems to be a casualty of RPC... */

    sz = 0;
    r = MsiGetPropertyA(hinst, "boo", NULL, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, sz == 6, "got size %u\n", sz);

    sz = 0;
    strcpy(buffer,"q");
    r = MsiGetPropertyA(hinst, "boo", buffer, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !strcmp(buffer, "q"), "got \"%s\"\n", buffer);
    todo_wine_ok(hinst, sz == 6, "got size %u\n", sz);

    sz = 1;
    strcpy(buffer,"x");
    r = MsiGetPropertyA(hinst, "boo", buffer, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !buffer[0], "got \"%s\"\n", buffer);
    todo_wine_ok(hinst, sz == 6, "got size %u\n", sz);

    sz = 3;
    strcpy(buffer,"x");
    r = MsiGetPropertyA(hinst, "boo", buffer, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !strcmp(buffer, "xy"), "got \"%s\"\n", buffer);
    todo_wine_ok(hinst, sz == 6, "got size %u\n", sz);

    sz = 4;
    strcpy(buffer,"x");
    r = MsiGetPropertyA(hinst, "boo", buffer, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, !strcmp(buffer, "xyz"), "got \"%s\"\n", buffer);
    ok(hinst, sz == 3, "got size %u\n", sz);

    sz = 0;
    r = MsiGetPropertyW(hinst, booW, NULL, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, sz == 3, "got size %u\n", sz);

    sz = 0;
    lstrcpyW(bufferW, booW);
    r = MsiGetPropertyW(hinst, booW, bufferW, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !lstrcmpW(bufferW, booW), "got %s\n", dbgstr_w(bufferW));
    ok(hinst, sz == 3, "got size %u\n", sz);

    sz = 1;
    lstrcpyW(bufferW, booW);
    r = MsiGetPropertyW(hinst, booW, bufferW, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !bufferW[0], "got %s\n", dbgstr_w(bufferW));
    ok(hinst, sz == 3, "got size %u\n", sz);

    sz = 3;
    lstrcpyW(bufferW, booW);
    r = MsiGetPropertyW(hinst, booW, bufferW, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !lstrcmpW(bufferW, xyW), "got %s\n", dbgstr_w(bufferW));
    ok(hinst, sz == 3, "got size %u\n", sz);

    sz = 4;
    lstrcpyW(bufferW, booW);
    r = MsiGetPropertyW(hinst, booW, bufferW, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, !lstrcmpW(bufferW, xyzW), "got %s\n", dbgstr_w(bufferW));
    ok(hinst, sz == 3, "got size %u\n", sz);

    r = MsiSetPropertyA(hinst, "boo", NULL);
    ok(hinst, !r, "got %u\n", r);
    check_prop(hinst, "boo", "");

    sz = 0;
    r = MsiGetPropertyA(hinst, "embednullprop", NULL, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, sz == 6, "got size %u\n", sz);

    sz = 4;
    memset(buffer, 0xcc, sizeof(buffer));
    r = MsiGetPropertyA(hinst, "embednullprop", buffer, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, sz == 3, "got size %u\n", sz);
    ok(hinst, !memcmp(buffer, "a\0\0\0\xcc", 5), "wrong data\n");
}

static void test_db(MSIHANDLE hinst)
{
    MSIHANDLE hdb, view, rec, rec2, suminfo;
    char buffer[10];
    DWORD sz;
    UINT r;

    hdb = MsiGetActiveDatabase(hinst);
    ok(hinst, hdb, "MsiGetActiveDatabase failed\n");

    r = MsiDatabaseIsTablePersistentA(hdb, "Test");
    ok(hinst, r == MSICONDITION_TRUE, "got %u\n", r);

    r = MsiDatabaseOpenViewA(hdb, NULL, &view);
    ok(hinst, r == ERROR_BAD_QUERY_SYNTAX, "got %u\n", r);

    r = MsiDatabaseOpenViewA(hdb, "SELECT * FROM `Test`", NULL);
    ok(hinst, r == ERROR_INVALID_PARAMETER, "got %u\n", r);

    r = MsiDatabaseOpenViewA(hdb, "SELECT * FROM `Test`", &view);
    ok(hinst, !r, "got %u\n", r);

    r = MsiViewGetColumnInfo(view, MSICOLINFO_NAMES, &rec2);
    ok(hinst, !r, "got %u\n", r);

    sz = sizeof(buffer);
    r = MsiRecordGetStringA(rec2, 1, buffer, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, sz == strlen(buffer), "got size %u\n", sz);
    ok(hinst, !strcmp(buffer, "Name"), "got '%s'\n", buffer);

    r = MsiCloseHandle(rec2);
    ok(hinst, !r, "got %u\n", r);

    r = MsiViewExecute(view, 0);
    ok(hinst, !r, "got %u\n", r);

    r = MsiViewFetch(view, &rec2);
    ok(hinst, !r, "got %u\n", r);

    r = MsiRecordGetFieldCount(rec2);
    ok(hinst, r == 3, "got %u\n", r);

    sz = sizeof(buffer);
    r = MsiRecordGetStringA(rec2, 1, buffer, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, sz == strlen(buffer), "got size %u\n", sz);
    ok(hinst, !strcmp(buffer, "one"), "got '%s'\n", buffer);

    r = MsiRecordGetInteger(rec2, 2);
    ok(hinst, r == 1, "got %d\n", r);

    sz = sizeof(buffer);
    r = MsiRecordReadStream(rec2, 3, buffer, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, !memcmp(buffer, "unus", 4), "wrong data\n");

    r = MsiCloseHandle(rec2);
    ok(hinst, !r, "got %u\n", r);

    r = MsiViewFetch(view, &rec2);
    ok(hinst, !r, "got %u\n", r);

    r = MsiRecordGetFieldCount(rec2);
    ok(hinst, r == 3, "got %u\n", r);

    sz = sizeof(buffer);
    r = MsiRecordGetStringA(rec2, 1, buffer, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, sz == strlen(buffer), "got size %u\n", sz);
    ok(hinst, !strcmp(buffer, "two"), "got '%s'\n", buffer);

    r = MsiRecordGetInteger(rec2, 2);
    ok(hinst, r == 2, "got %d\n", r);

    sz = sizeof(buffer);
    r = MsiRecordReadStream(rec2, 3, buffer, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, !memcmp(buffer, "duo", 3), "wrong data\n");

    r = MsiViewModify(view, MSIMODIFY_REFRESH, 0);
    ok(hinst, r == ERROR_INVALID_HANDLE, "got %u\n", r);

    r = MsiRecordSetStringA(rec2, 1, "three");
    ok(hinst, !r, "got %u\n", r);

    r = MsiRecordSetInteger(rec2, 2, 3);
    ok(hinst, !r, "got %u\n", r);

    r = MsiRecordSetInteger(rec2, 3, 3);
    ok(hinst, !r, "got %u\n", r);

    r = MsiViewModify(view, MSIMODIFY_REFRESH, rec2);
    ok(hinst, !r, "got %d\n", r);

    sz = sizeof(buffer);
    r = MsiRecordGetStringA(rec2, 1, buffer, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, sz == strlen(buffer), "got size %u\n", sz);
    ok(hinst, !strcmp(buffer, "two"), "got '%s'\n", buffer);

    r = MsiRecordGetInteger(rec2, 2);
    ok(hinst, r == 2, "got %d\n", r);

    sz = sizeof(buffer);
    r = MsiRecordReadStream(rec2, 3, buffer, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, !memcmp(buffer, "duo", 3), "wrong data\n");

    r = MsiCloseHandle(rec2);
    ok(hinst, !r, "got %u\n", r);

    r = MsiViewFetch(view, &rec2);
    ok(hinst, r == ERROR_NO_MORE_ITEMS, "got %u\n", r);
    ok(hinst, !rec2, "got %u\n", rec2);

    r = MsiViewClose(view);
    ok(hinst, !r, "got %u\n", r);

    r = MsiCloseHandle(view);
    ok(hinst, !r, "got %u\n", r);

    r = MsiDatabaseOpenViewA(hdb, "SELECT * FROM `Test` WHERE `Name` = ?", &view);
    ok(hinst, !r, "got %u\n", r);

    rec = MsiCreateRecord(1);
    MsiRecordSetStringA(rec, 1, "one");

    r = MsiViewExecute(view, rec);
    ok(hinst, !r, "got %u\n", r);

    r = MsiViewFetch(view, &rec2);
    ok(hinst, !r, "got %u\n", r);

    r = MsiRecordGetInteger(rec2, 2);
    ok(hinst, r == 1, "got %d\n", r);

    r = MsiCloseHandle(rec2);
    ok(hinst, !r, "got %u\n", r);

    r = MsiViewFetch(view, &rec2);
    ok(hinst, r == ERROR_NO_MORE_ITEMS, "got %u\n", r);
    ok(hinst, !rec2, "got %u\n", rec2);

    r = MsiCloseHandle(rec);
    ok(hinst, !r, "got %u\n", r);

    r = MsiCloseHandle(view);
    ok(hinst, !r, "got %u\n", r);

    /* test MsiDatabaseGetPrimaryKeys() */
    r = MsiDatabaseGetPrimaryKeysA(hdb, "Test", &rec);
    ok(hinst, !r, "got %u\n", r);

    r = MsiRecordGetFieldCount(rec);
    ok(hinst, r == 1, "got %d\n", r);

    sz = sizeof(buffer);
    r = MsiRecordGetStringA(rec, 0, buffer, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, sz == strlen(buffer), "got size %u\n", sz);
    ok(hinst, !strcmp(buffer, "Test"), "got '%s'\n", buffer);

    sz = sizeof(buffer);
    r = MsiRecordGetStringA(rec, 1, buffer, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, sz == strlen(buffer), "got size %u\n", sz);
    ok(hinst, !strcmp(buffer, "Name"), "got '%s'\n", buffer);

    r = MsiCloseHandle(rec);
    ok(hinst, !r, "got %u\n", r);

    r = MsiGetSummaryInformationA(hdb, NULL, 1, NULL);
    ok(hinst, r == ERROR_INVALID_PARAMETER, "got %u\n", r);

    r = MsiGetSummaryInformationA(hdb, NULL, 1, &suminfo);
    ok(hinst, !r, "got %u\n", r);

    r = MsiCloseHandle(suminfo);
    ok(hinst, !r, "got %u\n", r);

    r = MsiCloseHandle(hdb);
    ok(hinst, !r, "got %u\n", r);
}

static void test_doaction(MSIHANDLE hinst)
{
    UINT r;

    r = MsiDoActionA(hinst, "nested51");
    ok(hinst, !r, "got %u\n", r);
    check_prop(hinst, "nested", "1");

    r = MsiDoActionA(hinst, "nested1");
    ok(hinst, !r, "got %u\n", r);
    check_prop(hinst, "nested", "2");

    r = MsiSequenceA(hinst, NULL, 0);
    ok(hinst, r == ERROR_INVALID_PARAMETER, "got %u\n", r);

    r = MsiSequenceA(hinst, "TestSequence", 0);
    ok(hinst, !r, "got %u\n", r);
    check_prop(hinst, "nested", "1");
}

UINT WINAPI nested(MSIHANDLE hinst)
{
    MsiSetPropertyA(hinst, "nested", "2");

    return ERROR_SUCCESS;
}

static void test_targetpath(MSIHANDLE hinst)
{
    static const WCHAR targetdirW[] = {'T','A','R','G','E','T','D','I','R',0};
    static const WCHAR xyzW[] = {'C',':','\\',0};
    static const WCHAR xyW[] = {'C',':',0};
    WCHAR bufferW[100];
    char buffer[100];
    DWORD sz, srcsz;
    UINT r;

    /* test invalid values */
    r = MsiGetTargetPathA(hinst, NULL, NULL, NULL);
    ok(hinst, r == ERROR_INVALID_PARAMETER, "got %u\n", r);

    r = MsiGetTargetPathA(hinst, "TARGETDIR", NULL, NULL );
    ok(hinst, !r, "got %u\n", r);

    r = MsiGetTargetPathA(hinst, "TARGETDIR", buffer, NULL );
    ok(hinst, r == ERROR_INVALID_PARAMETER, "got %u\n", r);

    /* Returned size is in bytes, not chars, but only for custom actions.
     * Seems to be a casualty of RPC... */

    sz = 0;
    r = MsiGetTargetPathA(hinst, "TARGETDIR", NULL, &sz);
    ok(hinst, !r, "got %u\n", r);
    todo_wine_ok(hinst, sz == 6, "got size %u\n", sz);

    sz = 0;
    strcpy(buffer,"q");
    r = MsiGetTargetPathA(hinst, "TARGETDIR", buffer, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !strcmp(buffer, "q"), "got \"%s\"\n", buffer);
    todo_wine_ok(hinst, sz == 6, "got size %u\n", sz);

    sz = 1;
    strcpy(buffer,"x");
    r = MsiGetTargetPathA(hinst, "TARGETDIR", buffer, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !buffer[0], "got \"%s\"\n", buffer);
    todo_wine_ok(hinst, sz == 6, "got size %u\n", sz);

    sz = 3;
    strcpy(buffer,"x");
    r = MsiGetTargetPathA(hinst, "TARGETDIR", buffer, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !strcmp(buffer, "C:"), "got \"%s\"\n", buffer);
    todo_wine_ok(hinst, sz == 6, "got size %u\n", sz);

    sz = 4;
    strcpy(buffer,"x");
    r = MsiGetTargetPathA(hinst, "TARGETDIR", buffer, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, !strcmp(buffer, "C:\\"), "got \"%s\"\n", buffer);
    ok(hinst, sz == 3, "got size %u\n", sz);

    sz = 0;
    r = MsiGetTargetPathW(hinst, targetdirW, NULL, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, sz == 3, "got size %u\n", sz);

    sz = 0;
    bufferW[0] = 'q';
    r = MsiGetTargetPathW(hinst, targetdirW, bufferW, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, bufferW[0] == 'q', "got %s\n", dbgstr_w(bufferW));
    ok(hinst, sz == 3, "got size %u\n", sz);

    sz = 1;
    bufferW[0] = 'q';
    r = MsiGetTargetPathW(hinst, targetdirW, bufferW, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !bufferW[0], "got %s\n", dbgstr_w(bufferW));
    ok(hinst, sz == 3, "got size %u\n", sz);

    sz = 3;
    bufferW[0] = 'q';
    r = MsiGetTargetPathW(hinst, targetdirW, bufferW, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !lstrcmpW(bufferW, xyW), "got %s\n", dbgstr_w(bufferW));
    ok(hinst, sz == 3, "got size %u\n", sz);

    sz = 4;
    bufferW[0] = 'q';
    r = MsiGetTargetPathW(hinst, targetdirW, bufferW, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, !lstrcmpW(bufferW, xyzW), "got %s\n", dbgstr_w(bufferW));
    ok(hinst, sz == 3, "got size %u\n", sz);

    r = MsiSetTargetPathA(hinst, NULL, "C:\\subdir");
    ok(hinst, r == ERROR_INVALID_PARAMETER, "got %u\n", r);

    r = MsiSetTargetPathA(hinst, "TARGETDIR", NULL);
    ok(hinst, r == ERROR_INVALID_PARAMETER, "got %u\n", r);

    r = MsiSetTargetPathA(hinst, "TARGETDIR", "C:\\subdir");
    ok(hinst, !r, "got %u\n", r);

    sz = sizeof(buffer);
    r = MsiGetTargetPathA(hinst, "TARGETDIR", buffer, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, !strcmp(buffer, "C:\\subdir\\"), "got \"%s\"\n", buffer);

    r = MsiSetTargetPathA(hinst, "TARGETDIR", "C:\\");

    /* test GetSourcePath() */

    r = MsiGetSourcePathA(hinst, NULL, NULL, NULL);
    ok(hinst, r == ERROR_INVALID_PARAMETER, "got %u\n", r);

    r = MsiGetSourcePathA(hinst, "TARGETDIR", NULL, NULL );
    ok(hinst, !r, "got %u\n", r);

    r = MsiGetSourcePathA(hinst, "TARGETDIR", buffer, NULL );
    ok(hinst, r == ERROR_INVALID_PARAMETER, "got %u\n", r);

    /* Returned size is in bytes, not chars, but only for custom actions.
     * Seems to be a casualty of RPC... */

    srcsz = 0;
    MsiGetSourcePathW(hinst, targetdirW, NULL, &srcsz);

    sz = 0;
    r = MsiGetSourcePathA(hinst, "TARGETDIR", NULL, &sz);
    ok(hinst, !r, "got %u\n", r);
    todo_wine_ok(hinst, sz == srcsz * 2, "got size %u\n", sz);

    sz = 0;
    strcpy(buffer,"q");
    r = MsiGetSourcePathA(hinst, "TARGETDIR", buffer, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !strcmp(buffer, "q"), "got \"%s\"\n", buffer);
    todo_wine_ok(hinst, sz == srcsz * 2, "got size %u\n", sz);

    sz = 1;
    strcpy(buffer,"x");
    r = MsiGetSourcePathA(hinst, "TARGETDIR", buffer, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !buffer[0], "got \"%s\"\n", buffer);
    todo_wine_ok(hinst, sz == srcsz * 2, "got size %u\n", sz);

    sz = srcsz;
    strcpy(buffer,"x");
    r = MsiGetSourcePathA(hinst, "TARGETDIR", buffer, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, strlen(buffer) == srcsz - 1, "wrong buffer length %d\n", strlen(buffer));
    todo_wine_ok(hinst, sz == srcsz * 2, "got size %u\n", sz);

    sz = srcsz + 1;
    strcpy(buffer,"x");
    r = MsiGetSourcePathA(hinst, "TARGETDIR", buffer, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, strlen(buffer) == srcsz, "wrong buffer length %d\n", strlen(buffer));
    ok(hinst, sz == srcsz, "got size %u\n", sz);

    sz = 0;
    r = MsiGetSourcePathW(hinst, targetdirW, NULL, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, sz == srcsz, "got size %u\n", sz);

    sz = 0;
    bufferW[0] = 'q';
    r = MsiGetSourcePathW(hinst, targetdirW, bufferW, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, bufferW[0] == 'q', "got %s\n", dbgstr_w(bufferW));
    ok(hinst, sz == srcsz, "got size %u\n", sz);

    sz = 1;
    bufferW[0] = 'q';
    r = MsiGetSourcePathW(hinst, targetdirW, bufferW, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !bufferW[0], "got %s\n", dbgstr_w(bufferW));
    ok(hinst, sz == srcsz, "got size %u\n", sz);

    sz = srcsz;
    bufferW[0] = 'q';
    r = MsiGetSourcePathW(hinst, targetdirW, bufferW, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, lstrlenW(bufferW) == srcsz - 1, "wrong buffer length %d\n", lstrlenW(bufferW));
    ok(hinst, sz == srcsz, "got size %u\n", sz);

    sz = srcsz + 1;
    bufferW[0] = 'q';
    r = MsiGetSourcePathW(hinst, targetdirW, bufferW, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, lstrlenW(bufferW) == srcsz, "wrong buffer length %d\n", lstrlenW(bufferW));
    ok(hinst, sz == srcsz, "got size %u\n", sz);
}

static void test_misc(MSIHANDLE hinst)
{
    MSICONDITION cond;
    LANGID lang;
    UINT r;

    r = MsiSetMode(hinst, MSIRUNMODE_REBOOTATEND, FALSE);
    ok(hinst, !r, "got %u\n", r);

    lang = MsiGetLanguage(hinst);
    ok(hinst, lang == 1033, "got %u\n", lang);

    check_prop(hinst, "INSTALLLEVEL", "3");
    r = MsiSetInstallLevel(hinst, 123);
    ok(hinst, !r, "got %u\n", r);
    check_prop(hinst, "INSTALLLEVEL", "123");
    MsiSetInstallLevel(hinst, 3);

    cond = MsiEvaluateConditionA(hinst, NULL);
    ok(hinst, cond == MSICONDITION_NONE, "got %u\n", cond);
    MsiSetPropertyA(hinst, "condprop", "1");
    cond = MsiEvaluateConditionA(hinst, "condprop = 1");
    ok(hinst, cond == MSICONDITION_TRUE, "got %u\n", cond);
}

static void test_feature_states(MSIHANDLE hinst)
{
    INSTALLSTATE state, action;
    UINT r;

    /* test feature states */

    r = MsiGetFeatureStateA(hinst, NULL, &state, &action);
    ok(hinst, r == ERROR_UNKNOWN_FEATURE, "got %u\n", r);

    r = MsiGetFeatureStateA(hinst, "fake", &state, &action);
    ok(hinst, r == ERROR_UNKNOWN_FEATURE, "got %u\n", r);

    r = MsiGetFeatureStateA(hinst, "One", NULL, &action);
    ok(hinst, r == RPC_X_NULL_REF_POINTER, "got %u\n", r);

    r = MsiGetFeatureStateA(hinst, "One", &state, NULL);
    ok(hinst, r == RPC_X_NULL_REF_POINTER, "got %u\n", r);

    r = MsiGetFeatureStateA(hinst, "One", &state, &action);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, state == INSTALLSTATE_ABSENT, "got state %d\n", state);
    ok(hinst, action == INSTALLSTATE_LOCAL, "got action %d\n", action);

    r = MsiSetFeatureStateA(hinst, NULL, INSTALLSTATE_ABSENT);
    ok(hinst, r == ERROR_UNKNOWN_FEATURE, "got %u\n", r);

    r = MsiSetFeatureStateA(hinst, "One", INSTALLSTATE_ADVERTISED);
    ok(hinst, !r, "got %u\n", r);

    r = MsiGetFeatureStateA(hinst, "One", &state, &action);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, action == INSTALLSTATE_ADVERTISED, "got action %d\n", action);

    r = MsiSetFeatureStateA(hinst, "One", INSTALLSTATE_LOCAL);
    ok(hinst, !r, "got %u\n", r);

    r = MsiGetFeatureStateA(hinst, "One", &state, &action);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, action == INSTALLSTATE_LOCAL, "got action %d\n", action);

    /* test component states */

    r = MsiGetComponentStateA(hinst, NULL, &state, &action);
    ok(hinst, r == ERROR_UNKNOWN_COMPONENT, "got %u\n", r);

    r = MsiGetComponentStateA(hinst, "fake", &state, &action);
    ok(hinst, r == ERROR_UNKNOWN_COMPONENT, "got %u\n", r);

    r = MsiGetComponentStateA(hinst, "One", NULL, &action);
    ok(hinst, r == RPC_X_NULL_REF_POINTER, "got %u\n", r);

    r = MsiGetComponentStateA(hinst, "One", &state, NULL);
    ok(hinst, r == RPC_X_NULL_REF_POINTER, "got %u\n", r);

    r = MsiGetComponentStateA(hinst, "One", &state, &action);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, state == INSTALLSTATE_ABSENT, "got state %d\n", state);
    ok(hinst, action == INSTALLSTATE_LOCAL, "got action %d\n", action);

    r = MsiGetComponentStateA(hinst, "dangler", &state, &action);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, state == INSTALLSTATE_ABSENT, "got state %d\n", state);
    ok(hinst, action == INSTALLSTATE_UNKNOWN, "got action %d\n", action);

    r = MsiGetComponentStateA(hinst, "component", &state, &action);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, state == INSTALLSTATE_UNKNOWN, "got state %d\n", state);
    ok(hinst, action == INSTALLSTATE_LOCAL, "got action %d\n", action);

    r = MsiSetComponentStateA(hinst, NULL, INSTALLSTATE_ABSENT);
    ok(hinst, r == ERROR_UNKNOWN_COMPONENT, "got %u\n", r);

    r = MsiSetComponentStateA(hinst, "One", INSTALLSTATE_SOURCE);
    ok(hinst, !r, "got %u\n", r);

    r = MsiGetComponentStateA(hinst, "One", &state, &action);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, state == INSTALLSTATE_ABSENT, "got state %d\n", state);
    ok(hinst, action == INSTALLSTATE_SOURCE, "got action %d\n", action);

    r = MsiSetComponentStateA(hinst, "One", INSTALLSTATE_LOCAL);
    ok(hinst, !r, "got %u\n", r);

    r = MsiGetComponentStateA(hinst, "One", &state, &action);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, state == INSTALLSTATE_ABSENT, "got state %d\n", state);
    ok(hinst, action == INSTALLSTATE_LOCAL, "got action %d\n", action);
}

static void test_format_record(MSIHANDLE hinst)
{
    static const WCHAR xyzW[] = {'f','o','o',' ','1','2','3',0};
    static const WCHAR xyW[] = {'f','o','o',' ','1','2',0};
    WCHAR bufferW[10];
    char buffer[10];
    MSIHANDLE rec;
    DWORD sz;
    UINT r;

    r = MsiFormatRecordA(hinst, 0, NULL, NULL);
    ok(hinst, r == ERROR_INVALID_HANDLE, "got %u\n", r);

    rec = MsiCreateRecord(1);
    MsiRecordSetStringA(rec, 0, "foo [1]");
    MsiRecordSetInteger(rec, 1, 123);

    r = MsiFormatRecordA(hinst, rec, NULL, NULL);
    ok(hinst, !r, "got %u\n", r);

    r = MsiFormatRecordA(hinst, rec, buffer, NULL);
    ok(hinst, r == ERROR_INVALID_PARAMETER, "got %u\n", r);

    /* Returned size is in bytes, not chars, but only for custom actions. */

    sz = 0;
    r = MsiFormatRecordA(hinst, rec, NULL, &sz);
    ok(hinst, !r, "got %u\n", r);
    todo_wine_ok(hinst, sz == 14, "got size %u\n", sz);

    sz = 0;
    strcpy(buffer,"q");
    r = MsiFormatRecordA(hinst, rec, buffer, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !strcmp(buffer, "q"), "got \"%s\"\n", buffer);
    todo_wine_ok(hinst, sz == 14, "got size %u\n", sz);

    sz = 1;
    strcpy(buffer,"x");
    r = MsiFormatRecordA(hinst, rec, buffer, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !buffer[0], "got \"%s\"\n", buffer);
    todo_wine_ok(hinst, sz == 14, "got size %u\n", sz);

    sz = 7;
    strcpy(buffer,"x");
    r = MsiFormatRecordA(hinst, rec, buffer, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !strcmp(buffer, "foo 12"), "got \"%s\"\n", buffer);
    todo_wine_ok(hinst, sz == 14, "got size %u\n", sz);

    sz = 8;
    strcpy(buffer,"x");
    r = MsiFormatRecordA(hinst, rec, buffer, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, !strcmp(buffer, "foo 123"), "got \"%s\"\n", buffer);
    ok(hinst, sz == 7, "got size %u\n", sz);

    sz = 0;
    bufferW[0] = 'q';
    r = MsiFormatRecordW(hinst, rec, bufferW, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, bufferW[0] == 'q', "got %s\n", dbgstr_w(bufferW));
    ok(hinst, sz == 7, "got size %u\n", sz);

    sz = 1;
    bufferW[0] = 'q';
    r = MsiFormatRecordW(hinst, rec, bufferW, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !bufferW[0], "got %s\n", dbgstr_w(bufferW));
    ok(hinst, sz == 7, "got size %u\n", sz);

    sz = 7;
    bufferW[0] = 'q';
    r = MsiFormatRecordW(hinst, rec, bufferW, &sz);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !lstrcmpW(bufferW, xyW), "got %s\n", dbgstr_w(bufferW));
    ok(hinst, sz == 7, "got size %u\n", sz);

    sz = 8;
    bufferW[0] = 'q';
    r = MsiFormatRecordW(hinst, rec, bufferW, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, !lstrcmpW(bufferW, xyzW), "got %s\n", dbgstr_w(bufferW));
    ok(hinst, sz == 7, "got size %u\n", sz);

    /* check that properties work */
    MsiSetPropertyA(hinst, "fmtprop", "foobar");
    MsiRecordSetStringA(rec, 0, "[fmtprop]");
    sz = sizeof(buffer);
    r = MsiFormatRecordA(hinst, rec, buffer, &sz);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, !strcmp(buffer, "foobar"), "got \"%s\"\n", buffer);
    ok(hinst, sz == 6, "got size %u\n", sz);

    MsiCloseHandle(rec);
}

static void test_costs(MSIHANDLE hinst)
{
    static const WCHAR oneW[] = {'O','n','e',0};
    static const WCHAR xyzW[] = {'C',':',0};
    static const WCHAR xyW[] = {'C',0};
    WCHAR bufferW[10];
    char buffer[10];
    int cost, temp;
    DWORD sz;
    UINT r;

    cost = 0xdead;
    r = MsiGetFeatureCostA(hinst, NULL, MSICOSTTREE_CHILDREN, INSTALLSTATE_LOCAL, &cost);
    ok(hinst, r == ERROR_INVALID_PARAMETER, "got %u\n", r);
    todo_wine_ok(hinst, !cost, "got %d\n", cost);

    r = MsiGetFeatureCostA(hinst, "One", MSICOSTTREE_CHILDREN, INSTALLSTATE_LOCAL, NULL);
    ok(hinst, r == RPC_X_NULL_REF_POINTER, "got %u\n", r);

    cost = 0xdead;
    r = MsiGetFeatureCostA(hinst, "One", MSICOSTTREE_CHILDREN, INSTALLSTATE_LOCAL, &cost);
    ok(hinst, !r, "got %u\n", r);
    todo_wine_ok(hinst, cost == 8, "got %d\n", cost);

    sz = cost = temp = 0xdead;
    r = MsiEnumComponentCostsA(hinst, "One", 0, INSTALLSTATE_LOCAL, NULL, &sz, &cost, &temp);
    ok(hinst, r == ERROR_INVALID_PARAMETER, "got %u\n", r);
    ok(hinst, sz == 0xdead, "got size %d\n", sz);
    ok(hinst, cost == 0xdead, "got cost %d\n", cost);
    ok(hinst, temp == 0xdead, "got temp %d\n", temp);

    cost = temp = 0xdead;
    r = MsiEnumComponentCostsA(hinst, "One", 0, INSTALLSTATE_LOCAL, buffer, NULL, &cost, &temp);
    ok(hinst, r == ERROR_INVALID_PARAMETER, "got %u\n", r);
    ok(hinst, cost == 0xdead, "got cost %d\n", cost);
    ok(hinst, temp == 0xdead, "got temp %d\n", temp);

    sz = temp = 0xdead;
    r = MsiEnumComponentCostsA(hinst, "One", 0, INSTALLSTATE_LOCAL, buffer, &sz, NULL, &temp);
    ok(hinst, r == ERROR_INVALID_PARAMETER, "got %u\n", r);
    ok(hinst, sz == 0xdead, "got size %d\n", sz);
    ok(hinst, temp == 0xdead, "got temp %d\n", temp);

    sz = cost = 0xdead;
    r = MsiEnumComponentCostsA(hinst, "One", 0, INSTALLSTATE_LOCAL, buffer, &sz, &cost, NULL);
    ok(hinst, r == ERROR_INVALID_PARAMETER, "got %u\n", r);
    ok(hinst, sz == 0xdead, "got size %d\n", sz);
    ok(hinst, cost == 0xdead, "got cost %d\n", cost);

    cost = temp = 0xdead;
    sz = sizeof(buffer);
    r = MsiEnumComponentCostsA(hinst, NULL, 0, INSTALLSTATE_LOCAL, buffer, &sz, &cost, &temp);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, sz == 2, "got size %u\n", sz);
    ok(hinst, !strcmp(buffer, "C:"), "got '%s'\n", buffer);
    ok(hinst, !cost, "got cost %d\n", cost);
    ok(hinst, temp && temp != 0xdead, "got temp %d\n", temp);

    cost = temp = 0xdead;
    sz = sizeof(buffer);
    r = MsiEnumComponentCostsA(hinst, "One", 0, INSTALLSTATE_LOCAL, buffer, &sz, &cost, &temp);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, sz == 2, "got size %u\n", sz);
    ok(hinst, !strcmp(buffer, "C:"), "got '%s'\n", buffer);
    ok(hinst, cost == 8, "got cost %d\n", cost);
    ok(hinst, !temp, "got temp %d\n", temp);

    /* same string behaviour */
    cost = temp = 0xdead;
    sz = 0;
    strcpy(buffer,"q");
    r = MsiEnumComponentCostsA(hinst, "One", 0, INSTALLSTATE_LOCAL, buffer, &sz, &cost, &temp);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !strcmp(buffer, "q"), "got \"%s\"\n", buffer);
    todo_wine_ok(hinst, sz == 4, "got size %u\n", sz);
    ok(hinst, cost == 8, "got cost %d\n", cost);
    ok(hinst, !temp, "got temp %d\n", temp);

    sz = 1;
    strcpy(buffer,"x");
    r = MsiEnumComponentCostsA(hinst, "One", 0, INSTALLSTATE_LOCAL, buffer, &sz, &cost, &temp);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    todo_wine_ok(hinst, !buffer[0], "got \"%s\"\n", buffer);
    todo_wine_ok(hinst, sz == 4, "got size %u\n", sz);

    sz = 2;
    strcpy(buffer,"x");
    r = MsiEnumComponentCostsA(hinst, "One", 0, INSTALLSTATE_LOCAL, buffer, &sz, &cost, &temp);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    todo_wine_ok(hinst, !strcmp(buffer, "C"), "got \"%s\"\n", buffer);
    todo_wine_ok(hinst, sz == 4, "got size %u\n", sz);

    sz = 3;
    strcpy(buffer,"x");
    r = MsiEnumComponentCostsA(hinst, "One", 0, INSTALLSTATE_LOCAL, buffer, &sz, &cost, &temp);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, !strcmp(buffer, "C:"), "got \"%s\"\n", buffer);
    ok(hinst, sz == 2, "got size %u\n", sz);

    sz = 0;
    bufferW[0] = 'q';
    r = MsiEnumComponentCostsW(hinst, oneW, 0, INSTALLSTATE_LOCAL, bufferW, &sz, &cost, &temp);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, bufferW[0] == 'q', "got %s\n", dbgstr_w(bufferW));
    ok(hinst, sz == 2, "got size %u\n", sz);

    sz = 1;
    bufferW[0] = 'q';
    r = MsiEnumComponentCostsW(hinst, oneW, 0, INSTALLSTATE_LOCAL, bufferW, &sz, &cost, &temp);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !bufferW[0], "got %s\n", dbgstr_w(bufferW));
    ok(hinst, sz == 2, "got size %u\n", sz);

    sz = 2;
    bufferW[0] = 'q';
    r = MsiEnumComponentCostsW(hinst, oneW, 0, INSTALLSTATE_LOCAL, bufferW, &sz, &cost, &temp);
    ok(hinst, r == ERROR_MORE_DATA, "got %u\n", r);
    ok(hinst, !lstrcmpW(bufferW, xyW), "got %s\n", dbgstr_w(bufferW));
    ok(hinst, sz == 2, "got size %u\n", sz);

    sz = 3;
    bufferW[0] = 'q';
    r = MsiEnumComponentCostsW(hinst, oneW, 0, INSTALLSTATE_LOCAL, bufferW, &sz, &cost, &temp);
    ok(hinst, !r, "got %u\n", r);
    ok(hinst, !lstrcmpW(bufferW, xyzW), "got %s\n", dbgstr_w(bufferW));
    ok(hinst, sz == 2, "got size %u\n", sz);
}

/* Main test. Anything that doesn't depend on a specific install configuration
 * or have undesired side effects should go here. */
UINT WINAPI main_test(MSIHANDLE hinst)
{
    UINT res;
    IUnknown *unk = NULL;
    HRESULT hr;

    /* Test for an MTA apartment */
    hr = CoCreateInstance(&CLSID_XMLDocument, NULL, CLSCTX_INPROC_SERVER, &IID_IUnknown, (void **)&unk);
    ok(hinst, hr == S_OK, "CoCreateInstance failed with %08x\n", hr);

    if (unk) IUnknown_Release(unk);

    /* but ours is uninitialized */
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    ok(hinst, hr == S_OK, "got %#x\n", hr);
    CoUninitialize();

    /* Test MsiGetDatabaseState() */
    res = MsiGetDatabaseState(hinst);
    todo_wine_ok(hinst, res == MSIDBSTATE_ERROR, "expected MSIDBSTATE_ERROR, got %u\n", res);

    test_props(hinst);
    test_db(hinst);
    test_doaction(hinst);
    test_targetpath(hinst);
    test_misc(hinst);
    test_feature_states(hinst);
    test_format_record(hinst);
    test_costs(hinst);

    return ERROR_SUCCESS;
}

UINT WINAPI test_retval(MSIHANDLE hinst)
{
    char prop[10];
    DWORD len = sizeof(prop);
    UINT retval;

    MsiGetPropertyA(hinst, "TEST_RETVAL", prop, &len);
    sscanf(prop, "%u", &retval);
    return retval;
}

static void append_file(MSIHANDLE hinst, const char *filename, const char *text)
{
    DWORD size;
    HANDLE file = CreateFileA(filename, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    ok(hinst, file != INVALID_HANDLE_VALUE, "CreateFile failed, error %u\n", GetLastError());

    SetFilePointer(file, 0, NULL, FILE_END);
    WriteFile(file, text, strlen(text), &size, NULL);
    CloseHandle(file);
}

UINT WINAPI da_immediate(MSIHANDLE hinst)
{
    char prop[300];
    DWORD len = sizeof(prop);

    MsiGetPropertyA(hinst, "TESTPATH", prop, &len);

    append_file(hinst, prop, "one");

    ok(hinst, !MsiGetMode(hinst, MSIRUNMODE_SCHEDULED), "shouldn't be scheduled\n");
    ok(hinst, !MsiGetMode(hinst, MSIRUNMODE_ROLLBACK), "shouldn't be rollback\n");
    ok(hinst, !MsiGetMode(hinst, MSIRUNMODE_COMMIT), "shouldn't be commit\n");

    return ERROR_SUCCESS;
}

UINT WINAPI da_deferred(MSIHANDLE hinst)
{
    char prop[300];
    DWORD len = sizeof(prop);
    LANGID lang;
    UINT r;

    /* Test that we were in fact deferred */
    r = MsiGetPropertyA(hinst, "CustomActionData", prop, &len);
    ok(hinst, r == ERROR_SUCCESS, "got %u\n", r);
    ok(hinst, prop[0], "CustomActionData was empty\n");

    append_file(hinst, prop, "two");

    /* Test available properties */
    len = sizeof(prop);
    r = MsiGetPropertyA(hinst, "ProductCode", prop, &len);
    ok(hinst, r == ERROR_SUCCESS, "got %u\n", r);
    ok(hinst, prop[0], "got %s\n", prop);

    len = sizeof(prop);
    r = MsiGetPropertyA(hinst, "UserSID", prop, &len);
    ok(hinst, r == ERROR_SUCCESS, "got %u\n", r);
    ok(hinst, prop[0], "got %s\n", prop);

    len = sizeof(prop);
    r = MsiGetPropertyA(hinst, "TESTPATH", prop, &len);
    ok(hinst, r == ERROR_SUCCESS, "got %u\n", r);
    todo_wine_ok(hinst, !prop[0], "got %s\n", prop);

    /* Test modes */
    ok(hinst, MsiGetMode(hinst, MSIRUNMODE_SCHEDULED), "should be scheduled\n");
    ok(hinst, !MsiGetMode(hinst, MSIRUNMODE_ROLLBACK), "shouldn't be rollback\n");
    ok(hinst, !MsiGetMode(hinst, MSIRUNMODE_COMMIT), "shouldn't be commit\n");

    lang = MsiGetLanguage(hinst);
    ok(hinst, lang != ERROR_INVALID_HANDLE, "MsiGetLanguage failed\n");

    return ERROR_SUCCESS;
}
