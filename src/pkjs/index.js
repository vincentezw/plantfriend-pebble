const Clay = require('@rebble/clay');
const clayConfig = require('./config');
const PlantFriendApi = require("./plantfriendApi.js");
const {capitalise, formatDate} = require("./helpers");
const clay = new Clay(clayConfig, null, {autoHandleEvents: false});

let plantfriendToken;
let householdId = parseInt(localStorage.getItem("householdId"));

function getClaySettings() {
  try {
    return JSON.parse(localStorage.getItem('clay-settings')) || {};
  } catch (e) {
    console.log('Failed to parse Clay settings:', e.toString());
    return {};
  }
}

function loadSettings() {
  const claySettings = getClaySettings();
  plantfriendToken = claySettings.Token || '';
}

let state = {
  actions: null,
  tasks: null,
};

const menuEnum = {
  main: 0,
  tasks: 1,
  actions: 2,
  household: 3,
  handleTask: 4,
  handleAction: 5,
};

loadSettings();
const api = new PlantFriendApi(
  plantfriendToken
);

Pebble.addEventListener('ready', function (_e) {
  if (householdId) {
    api.getActionsAndTasks(householdId, getTasksAndActionsCallbacK);
  } else {
    sendMainMenu();
  }
});

function sendErrorMessage(message) {
  Pebble.sendAppMessage({
    cmd: 3,
    data: message,
  });
}

function sendHouseholdMenu(result) {
  if (result.error) {
    console.error("Error getting households", result.error);
    sendErrorMessage(result.error);
    return;
  }

  const data = result.data;
  const currentHouseholdId = data.currentHousehold ? data.currentHousehold.id : null;
  const menu = data.households.map((household) => ({
    title: capitalise(household.name),
    subtitle: household.id === currentHouseholdId ? "Current household" : "",
    id: household.id,
    icon: 3,
  }));

  Pebble.sendAppMessage({
    cmd: 0,
    data: formatForMenu(menu),
    cur_menu: menuEnum.household,
  });
}

function getTasksAndActionsCallbacK(result) {
  if (result.error) {
    console.error("Error getting actions and tasks", result.error);
    sendErrorMessage(result.error);
    return;
  }

  if (result.data) {
    state.actions = result.data.actions;
    state.tasks = result.data.tasks;
  }
  sendMainMenu();
}

function getActionsCallback(result) {
  if (result.error) {
    console.error("Error getting actions", data.error);
    sendErrorMessage(result.error);
    return;
  }

  state.actions = result.data;
  sendActionsMenu(true);
}

function getTasksCallback(result) {
  if (result.error) {
    console.error("Error getting tasks", data.error);
    sendErrorMessage(result.error);
    return;
  }

  state.tasks = result.data;
  sendTasksMenu(true);
}

function handleActionCallback() {
  Pebble.sendAppMessage({
    cmd: 2,
    data: null,
    cur_menu: menuEnum.handleAction,
  });
}

function handleTaskCallback() {
  Pebble.sendAppMessage({
    cmd: 2,
    data: null,
    cur_menu: menuEnum.handleTask,
  });
}

function sendMainMenu() {
  const menu = [
    {
      title: "Household",
      subtitle: "Select household to view",
      id: "household",
      icon: 3,
    }
  ];

  if (householdId) {
    menu.unshift(
      {
        title: "Tasks",
        subtitle: state.tasks.totalCount + " pending",
        id: "tasks",
        icon: 1,
      },
      {
        title: "Actions",
        subtitle: state.actions.totalCount + " performed today",
        id: "actions",
        icon: 2,
      }
    );
  }

  const data = formatForMenu(menu);
  Pebble.sendAppMessage({
    cmd: 0,
    cur_menu: menuEnum.main,
    data,
  });
}

function sendActionsMenu() {
  const actionTitle = (action) => {
    console.log("action", JSON.stringify(action));
    if (action.type === "snooze") {
      return "Snooze " + action.details.type + " " + action.plantName;
    }
    return capitalise(action.type) + " " + action.plantName;
  }

  const menu = state.actions.items.map((action) => ({
    title: actionTitle(action),
    subtitle: action.familyName + " (" + action.roomName + ")",
    id: action.id + "_" + action.type,
    icon: getIconForActionType(action.type),
  }));

  if (state.actions.hasPrevPage) {
    menu.unshift({
      title: "Previous page",
      subtitle: "On page " + state.actions.currentPage + " of " + state.actions.totalPages,
      id: "prev",
      icon: 9,
    });
  }
  if (state.actions.hasNextPage) {
    menu.push({
      title: "Next page",
      subtitle: "On page " + state.actions.currentPage + " of " + state.actions.totalPages,
      id: "next",
      icon: 8,
    });
  }

  const data = formatForMenu(menu);
  Pebble.sendAppMessage({
    cmd: 0,
    data,
    cur_menu: menuEnum.actions,
  });
}

function sendTasksMenu() {
  const menu = state.tasks.items.map((task) => ({
    title: capitalise(task.type) + " " + task.plantName,
    subtitle: task.familyName + " (" + task.roomName + ")",
    id: task.id + "_" + task.plantId + "_" + task.type,
    icon: getIconForActionType(task.type),
  }));

  if (state.tasks.hasPrevPage) {
    menu.unshift({
      title: "Previous page",
      subtitle: "On page " + state.tasks.currentPage + " of " + state.tasks.totalPages,
      id: "prev",
      icon: 9,
    });
  }
  if (state.tasks.hasNextPage) {
    menu.push({
      title: "Next page",
      subtitle: "On page " + state.tasks.currentPage + " of " + state.tasks.totalPages,
      id: "next",
      icon: 8,
    });
  }

  const data = formatForMenu(menu);
  Pebble.sendAppMessage({
    cmd: 0,
    data,
    cur_menu: menuEnum.tasks,
  });
}

function sendHandleTaskMenu(taskData) {
  const menu = [
    {
      title: "Mark as done",
      subtitle: "Mark this task as completed",
      id: "done" + "_" + taskData,
      icon: 10,
    },
  ];

  const snoozeTimes = [2, 7, 14];
  for (let i = 0; i < snoozeTimes.length; i++) {
    const days = snoozeTimes[i];
    const today = new Date();
    const snoozeDate = new Date(today.getTime() + days * 24 * 60 * 60 * 1000);

    menu.push({
      title: "Snooze for " + days + " days",
      subtitle: "Delay till " + formatDate(snoozeDate),
      id: "snooze_" + taskData + "_" + days,
      icon: 11,
    });
  }
  
  const data = formatForMenu(menu);
  Pebble.sendAppMessage({
    cmd: 0,
    data,
    cur_menu: menuEnum.handleTask,
  });
}

function sendHandleActionMenu(actionData) {
  const menu = [
    {
      title: "Undo",
      subtitle: "Mark this task as not completed",
      id: "undo" + "_" + actionData,
      icon: 12,
    },
  ];
  
  const data = formatForMenu(menu);
  Pebble.sendAppMessage({
    cmd: 0,
    data,
    cur_menu: menuEnum.handleAction,
  });
}

function formatForMenu(items) {
  return items.map(item =>
    [
      item.id,
      item.title,
      item.subtitle,
      item.icon
    ].join("|")
  ).join("\n");
}

Pebble.addEventListener('showConfiguration', function(e) {
  loadSettings();
  Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response) return;

  let dict;
  try {
    dict = clay.getSettings(e.response, false);
  } catch (err) {
    console.log('Failed to parse config response:', err.toString());
    return;
  }

  plantfriendUrl = (dict.Url && dict.Url.value) || '';
  plantfriendToken = (dict.Token && dict.Token.value) || '';

  clay.setSettings({
    Url: plantfriendUrl,
    Token: plantfriendToken,
  });
  api.updateSettings(plantfriendUrl, plantfriendToken);
});

Pebble.addEventListener('appmessage', function (e) {
  const cmd = e.payload.cmd;
 
  // Refresh
  if (cmd === 5) {
    const menu = e.payload.cur_menu;
    switch (menu) {
      case menuEnum.main:
        api.getActionsAndTasks(householdId, getTasksAndActionsCallbacK);
        break;
      case menuEnum.tasks:
        api.getTasks(householdId, state.tasks.currentPage, getTasksCallback);
        break;
      case menuEnum.actions:
        api.getActions(householdId, state.actions.currentPage, getActionsCallback);
        break;
    }
    return;
  }

  if (cmd !== 1) {
    return;
  }

  const menu = e.payload.cur_menu;
  const selectedId = e.payload.data;
  if (menu === menuEnum.main) {
    switch (selectedId) {
      case "tasks":
        sendTasksMenu();
        break;
      case "actions":
        sendActionsMenu();
        break;
      case "household":
        api.getUserHouseholds(sendHouseholdMenu);
        break;
    }
  }

  if (menu === menuEnum.actions) {
    if (selectedId === "next" || selectedId === "prev") {
      let page = selectedId === "next"
        ? state.actions.currentPage + 1
        : state.actions.currentPage - 1;
      page = Math.max(0, Math.min(page, state.actions.totalPages));
      api.getActions(householdId, page, getActionsCallback);
      return;
    }

    sendHandleActionMenu(selectedId);
  }

  if (menu === menuEnum.tasks) {
    if (selectedId === "next" || selectedId === "prev") {
      let page = selectedId === "next"
        ? state.tasks.currentPage + 1
        : state.tasks.currentPage - 1;
      page = Math.max(0, Math.min(page, state.tasks.totalPages));
      api.getTasks(householdId, page, getTasksCallback);
      return;
    }

    sendHandleTaskMenu(selectedId);
  }

  if (menu === menuEnum.handleTask) {
    const [action, id, plantId, type, days] = selectedId.split("_");
    if (action === "done") {
      api.markTaskComplete(handleTaskCallback, parseInt(id));
    } else if (action === "snooze") {
      api.snoozeTask(handleTaskCallback, parseInt(id), parseInt(plantId), type, parseInt(days));
    }
  }

  if (menu === menuEnum.handleAction) {
    const [action, id, _type] = selectedId.split("_");
    if (action === "undo") {
      api.deleteAction(handleActionCallback, parseInt(id));
    }
  }
  
  if (menu === menuEnum.household) {
    householdId = parseInt(selectedId);
    localStorage.setItem("householdId", householdId);
    // refresh main menu?
    Pebble.sendAppMessage({
      cmd: 2,
      data: null,
      cur_menu: menuEnum.household,
    });
  }
});

function getIconForActionType(type) {
  switch (type) {
    case "water":
      return 4;
    case "fertilise":
      return 5;
    case "clean":
      return 6;
    case "mist":
      return 7;
    case "snooze":
      return 11;
    default:
      return 0;
  }
}
