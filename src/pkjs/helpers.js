function ordinal(n) {
  const mod10 = n % 10;
  const mod100 = n % 100;

  if (mod10 === 1 && mod100 !== 11) return n + "st";
  if (mod10 === 2 && mod100 !== 12) return n + "nd";
  if (mod10 === 3 && mod100 !== 13) return n + "rd";

  return n + "th";
}

function capitalise(str) {
  if (!str) { return str; }
  return str.charAt(0).toUpperCase() + str.slice(1);
}

function formatDate(date) {
  const weekdays = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"];
  const months = [
    "January", "February", "March", "April",
    "May", "June", "July", "August",
    "September", "October", "November", "December"
  ];

  return `${weekdays[date.getDay()]}, ${months[date.getMonth()]} ${ordinal(date.getDate())}`;
}

module.exports = {
  capitalise,
  formatDate,
};
