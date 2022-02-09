/*
 * Copyright (c) 2021, Jan de Visser <jan@de-visser.net>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibSQL/AST/AST.h>
#include <LibSQL/Database.h>
#include <LibSQL/Meta.h>

namespace SQL::AST {

Result CreateSchema::execute(ExecutionContext& context) const
{
    auto schema_def = TRY(context.database->get_schema(m_schema_name));

    if (schema_def) {
        if (m_is_error_if_schema_exists)
            return { SQLCommand::Create, SQLErrorCode::SchemaExists, m_schema_name };
        return { SQLCommand::Create };
    }

    schema_def = SchemaDef::construct(m_schema_name);
    TRY(context.database->add_schema(*schema_def));

    return { SQLCommand::Create, 0, 1 };
}

}
