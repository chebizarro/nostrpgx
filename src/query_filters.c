/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "postgres.h"
#include "fmgr.h"
#include "executor/spi.h"
#include "funcapi.h"
#include "utils/jsonb.h"
#include "utils/builtins.h"
#include "utils/elog.h"
#include "utils/lsyscache.h"
#include "access/htup_details.h"
#include "utils/memutils.h"

extern int work_mem;

PG_FUNCTION_INFO_V1(query_filters);

Datum query_filters(PG_FUNCTION_ARGS)
{
  Jsonb *filters_jsonb = PG_GETARG_JSONB_P(0);
  bool return_count = PG_GETARG_BOOL(1);
  JsonbIterator *it;
  JsonbValue v;
  JsonbIteratorToken type;

  StringInfoData query;
  initStringInfo(&query);

  if (return_count)
  {
    appendStringInfoString(&query, "SELECT COUNT(*) FROM events e WHERE ");
  }
  else
  {
    appendStringInfoString(&query, "SELECT e.* FROM events e WHERE ");
  }

  bool has_conditions = false;
  char *search_query = NULL;

  it = JsonbIteratorInit(&filters_jsonb->root);

  while ((type = JsonbIteratorNext(&it, &v, false)) != WJB_DONE)
  {
    if (type == WJB_KEY)
    {
      if (strcmp(v.val.string.val, "ids") == 0)
      {
        JsonbIteratorNext(&it, &v, false);
        appendStringInfoString(&query, "e.event_id = ANY(");
        JsonbIterator *subit = JsonbIteratorInit(v.val.binary.data);
        JsonbValue subv;
        appendStringInfoChar(&query, '(');
        bool first = true;

        while ((type = JsonbIteratorNext(&subit, &subv, false)) != WJB_DONE)
        {
          if (type == WJB_ELEM && subv.type == jbvString)
          {
            if (first)
            {
              first = false;
            }
            else
            {
              appendStringInfoChar(&query, ',');
            }
            appendStringInfo(&query, "'%s'", subv.val.string.val);
          }
        }

        appendStringInfoString(&query, ")) AND ");
        has_conditions = true;
      }
      else if (strcmp(v.val.string.val, "authors") == 0)
      {
        JsonbIteratorNext(&it, &v, false);
        appendStringInfoString(&query, "e.author = ANY(");
        JsonbIterator *subit = JsonbIteratorInit(v.val.binary.data);
        JsonbValue subv;
        appendStringInfoChar(&query, '(');
        bool first = true;

        while ((type = JsonbIteratorNext(&subit, &subv, false)) != WJB_DONE)
        {
          if (type == WJB_ELEM && subv.type == jbvString)
          {
            if (first)
            {
              first = false;
            }
            else
            {
              appendStringInfoChar(&query, ',');
            }
            appendStringInfo(&query, "'%s'", subv.val.string.val);
          }
        }

        appendStringInfoString(&query, ")) AND ");
        has_conditions = true;
      }
      else if (strcmp(v.val.string.val, "kinds") == 0)
      {
        JsonbIteratorNext(&it, &v, false);
        appendStringInfoString(&query, "e.kind = ANY(");
        JsonbIterator *subit = JsonbIteratorInit(v.val.binary.data);
        JsonbValue subv;
        appendStringInfoChar(&query, '(');
        bool first = true;

        while ((type = JsonbIteratorNext(&subit, &subv, false)) != WJB_DONE)
        {
          if (type == WJB_ELEM && subv.type == jbvNumeric)
          {
            if (first)
            {
              first = false;
            }
            else
            {
              appendStringInfoChar(&query, ',');
            }
            appendStringInfo(&query, "%s", DatumGetCString(DirectFunctionCall1(numeric_out, NumericGetDatum(subv.val.numeric))));
          }
        }

        appendStringInfoString(&query, ")) AND ");
        has_conditions = true;
      }
      else if (strcmp(v.val.string.val, "since") == 0)
      {
        JsonbIteratorNext(&it, &v, false);
        appendStringInfo(&query, "e.created_at > %s AND ", DatumGetCString(DirectFunctionCall1(int8out, Int64GetDatum(DatumGetInt64(DirectFunctionCall1(numeric_int8, NumericGetDatum(v.val.numeric)))))));
        has_conditions = true;
      }
      else if (strcmp(v.val.string.val, "until") == 0)
      {
        JsonbIteratorNext(&it, &v, false);
        appendStringInfo(&query, "e.created_at < %s AND ", DatumGetCString(DirectFunctionCall1(int8out, Int64GetDatum(DatumGetInt64(DirectFunctionCall1(numeric_int8, NumericGetDatum(v.val.numeric)))))));
        has_conditions = true;
      }
      else if (strcmp(v.val.string.val, "limit") == 0)
      {
        JsonbIteratorNext(&it, &v, false);
        appendStringInfo(&query, "LIMIT %s", DatumGetCString(DirectFunctionCall1(numeric_out, NumericGetDatum(v.val.numeric))));
      }
      else if (strcmp(v.val.string.val, "search") == 0)
      {
        JsonbIteratorNext(&it, &v, false);
        search_query = v.val.string.val;
      }
      else if (v.val.string.val[0] == '#')
      {
        JsonbIteratorNext(&it, &v, false);
        appendStringInfo(&query, "e.event_id IN (SELECT event_id FROM tags WHERE tag_name = '%s' AND tag_value = ANY(", v.val.string.val + 1);
        JsonbIterator *subit = JsonbIteratorInit(v.val.binary.data);
        JsonbValue subv;
        appendStringInfoChar(&query, '(');
        bool first = true;

        while ((type = JsonbIteratorNext(&subit, &subv, false)) != WJB_DONE)
        {
          if (type == WJB_ELEM && subv.type == jbvString)
          {
            if (first)
            {
              first = false;
            }
            else
            {
              appendStringInfoChar(&query, ',');
            }
            appendStringInfo(&query, "'%s'", subv.val.string.val);
          }
        }

        appendStringInfoString(&query, ")) AND ");
        has_conditions = true;
      }
    }
  }

  if (search_query != NULL)
  {
    appendStringInfo(&query, "to_tsvector('english', e.content) @@ plainto_tsquery('english', '%s') AND ", search_query);
    has_conditions = true;
  }

  if (has_conditions)
  {
    query.len -= 5; // Remove the trailing " AND "
  }

  SPI_connect();
  int ret = SPI_exec(query.data, 0);
  if (ret != SPI_OK_SELECT)
  {
    SPI_finish();
    ereport(ERROR, (errmsg("SPI_exec failed: %s", SPI_result_code_string(ret))));
  }

  if (return_count)
  {
    int32 count = 0;
    if (SPI_processed > 0)
    {
      count = DatumGetInt32(SPI_getbinval(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, 1, NULL));
    }
    SPI_finish();
    PG_RETURN_INT32(count);
  }
  else
  {
    TupleDesc tupdesc = SPI_tuptable->tupdesc;
    SPITupleTable *tuptable = SPI_tuptable;

    int numrows = SPI_processed;
    Datum *values;
    bool *nulls;
    HeapTuple tuple;
    int i;

    Tuplestorestate *tuplestorestate = NULL;
    MemoryContext oldcontext = MemoryContextSwitchTo(CurrentMemoryContext);
    tuplestorestate = tuplestore_begin_heap(true, false, work_mem);
    MemoryContextSwitchTo(oldcontext);

    for (i = 0; i < numrows; i++)
    {
      tuple = tuptable->vals[i];
      values = SPI_palloc(sizeof(Datum) * tupdesc->natts);
      nulls = SPI_palloc(sizeof(bool) * tupdesc->natts);
      heap_deform_tuple(tuple, tupdesc, values, nulls);
      tuplestore_putvalues(tuplestorestate, tupdesc, values, nulls);
      SPI_pfree(values);
      SPI_pfree(nulls);
    }

    SPI_finish();
    tuplestore_donestoring(tuplestorestate);

    PG_RETURN_NULL();
  }
}
