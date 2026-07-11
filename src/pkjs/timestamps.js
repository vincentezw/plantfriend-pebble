function getDayRange(offsetDays) {
  const d = new Date();
  d.setUTCDate(d.getUTCDate() + offsetDays);

  const from = new Date(Date.UTC(
    d.getUTCFullYear(),
    d.getUTCMonth(),
    d.getUTCDate(),
    0, 0, 0, 0
  ));

  const to = new Date(Date.UTC(
    d.getUTCFullYear(),
    d.getUTCMonth(),
    d.getUTCDate(),
    23, 59, 59, 999
  ));

  return {
    fromDate: from.toISOString(),
    toDate: to.toISOString()
  };
}

module.exports = {
  getDayRange
};
