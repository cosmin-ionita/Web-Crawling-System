#include "Client.h"

bool Client::operator>(const Client c) const
{
    return this->availability < c.availability;
}
