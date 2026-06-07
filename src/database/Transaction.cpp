#include "../../include/database/Transaction.hpp"
#include "../../include/database/Database.hpp"

Transaction::Transaction(
    Database& db
)
    : db_(db),
      finalized_(false)
{
    db_.execute(
            "BEGIN TRANSACTION;"
            );
}

void Transaction::commit()
{
    db_.execute(
        "COMMIT;"
    );

    finalized_ = true;
}


void Transaction::rollback()
{
    db_.execute(
        "ROLLBACK;"
    );

    finalized_ = true;
}

Transaction::~Transaction()
{
    if (!finalized_)

    {
        rollback();
    }
}
