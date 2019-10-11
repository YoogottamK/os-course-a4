#include <stdbool.h>

#include "main.h"

#ifndef __RIDER_H
#define __RIDER_H

/*
 * This function books a cab of type t, will wait
 *  for atmost maxWaitTime for the booking, and
 *  if cab is succesfully booked, the ride will
 *  take rideTime seconds to complete
 *
 * Returns whether cab booking was succesful or not
 */
bool bookCab(Rider * r);

void initRider(int uid);

#endif // __RIDER_H
