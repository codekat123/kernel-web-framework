#include "../../include/database/Transaction.hpp"
#include "../../include/database/Database.hpp"

Transaction::Transaction(
    Database& db
)
    : db_(db),
      committed_(false)
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

    committed_ = true;
}


void Transaction::rollback()
{
    db_.execute(
        "ROLLBACK;"
    );

    committed_ = true;
}

Transaction::~Transaction()
{
    if (!committed_)

    {
        rollback();
    }
}
