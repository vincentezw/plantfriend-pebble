const Timestamps = require("./timestamps");

const ITEMS_PER_PAGE = 5;
const PLANTFRIEND_URL = "https://myplantfriend.com";

const procedures = {
  createAction: "action.create",
  deleteAction: "action.delete",
  editSchedule: "schedule.edit",
  markDone: "schedule.markDone",
  listActions: "action.list",
  listSchedules: "schedule.list",
  getUserHouseholds: "household.getUserHouseholds"
}

const QueryType = {
  QUERY_TYPE_PAGE: 0,
  QUERY_TYPE_ITEM: 1,
  MUTATION_TYPE: 2,
}

class PlantFriendApi {
  constructor(token) {
    this._token = token;
  }

  updateSettings(url, token) {
    this._token = token;
  }

  getActionsAndTasks(householdId, callback) {
    const procedure = [procedures.listActions, procedures.listSchedules];
    const url = this._buildTrpcUrl(procedure);
    const input = {
      "batch": 1,
      "input": JSON.stringify({
        0: this._createQueryActionsInputs(householdId, 1),
        1: this._createQueryScheduleInputs(householdId, 1),
      }),
    };

    this._makeRequest(url, input, QueryType.QUERY_TYPE_PAGE, callback);
  }

  getActions(householdId, page, callback) {
    const procedure = [procedures.listActions];
    const url = this._buildTrpcUrl(procedure);
    const input = {
      "batch": 1,
      "input": JSON.stringify({
        0: this._createQueryActionsInputs(householdId, page),
      }),
    };

    this._makeRequest(url, input, QueryType.QUERY_TYPE_PAGE, callback);
  }

  getTasks(householdId, page, callback) {
    const procedure = [procedures.listSchedules];
    const url = this._buildTrpcUrl(procedure);
    const input = {
      "batch": 1,
      "input": JSON.stringify({
        0: this._createQueryScheduleInputs(householdId, page),
      }),
    };

    this._makeRequest(url, input, QueryType.QUERY_TYPE_PAGE, callback);
  }

  markTaskComplete(callback, scheduleId) {
    const procedure = [procedures.markDone];
    const url = this._buildTrpcUrl(procedure) + "?batch=1";

    const input = {
      "0": {
        "id": scheduleId,
      }
    };
    this._makeRequest(url, input, QueryType.MUTATION_TYPE, callback);
  }

  snoozeTask(callback, scheduleId, plantId, scheduleType, days) {
    const durationMs = days * 24 * 60 * 60 * 1000;
    const scheduledForMs = Date.now() + durationMs;

    const procedure = [procedures.editSchedule, procedures.createAction];
    const url = this._buildTrpcUrl(procedure) + "?batch=1";
    const input = {
      "0": {
        "id": scheduleId,
        "nextScheduledAt": scheduledForMs,
      },
      "1": {
        "details": {
          "duration": days,
          "type": scheduleType,
        },
        "type": "snooze",
        "plantId": plantId,
      },
    };

    this._makeRequest(url, input, QueryType.MUTATION_TYPE, callback);
  }

  deleteAction(callback, actionId) {
    const procedure = [procedures.deleteAction];
    const url = this._buildTrpcUrl(procedure) + "?batch=1";
    const input = {
      "0": {
        "id": actionId,
      },
    };

    this._makeRequest(url, input, QueryType.MUTATION_TYPE, callback);
  }

  _buildTrpcUrl(procedures) {
    const joinedProcedures = procedures.join(",");
    const base = PLANTFRIEND_URL + "/api/trpc/";

    return base + joinedProcedures;
  }

  getUserHouseholds(callback) {
    const procedure = [procedures.getUserHouseholds];
    const url = this._buildTrpcUrl(procedure);
    const input = {
      "batch": 1,
    };

    this._makeRequest(url, input, QueryType.QUERY_TYPE_ITEM, callback);
  }

  _createQueryActionsInputs(householdId, page) {
    return {
      "fromDate": Timestamps.getDayRange(0).fromDate,
      "toDate": Timestamps.getDayRange(0).toDate,
      "householdId": householdId,
      "page": page,
      "limit": ITEMS_PER_PAGE,
    };
  }

  _createQueryScheduleInputs(householdId, page) {
    return {
      "householdId": householdId,
      "onlyOverdue": true,
      "status": "pending",
      "page": page,
      "limit": ITEMS_PER_PAGE,
    };
  }

  _makeRequest(url, input, type, callback) {
    if (!this._token) {
      callback({
        data: null,
        error: "Plantfriend is not configured.",
      });
      return;
    }

    const isMutation = type === QueryType.MUTATION_TYPE;
    const xhr = new XMLHttpRequest();

    if (isMutation) {
      xhr.open("POST", url, true);
    } else {
      const query =
        "?batch=" + encodeURIComponent(input.batch) +
        "&input=" + encodeURIComponent(input.input);

      xhr.open("GET", url + query, true);
    }
    xhr.setRequestHeader("Authorization", "Bearer " + this._token);
    if (isMutation) {
      xhr.setRequestHeader("Content-Type", "application/json");
    }
    xhr.onload = () => {
      if (xhr.status >= 200 && xhr.status < 300) {
        const response = JSON.parse(xhr.responseText);
        if (response.length === 2) {
          callback({
            data: {
              actions: response[0].result.data,
              tasks: response[1].result.data,
            },
            error: null,
          });
        } else {
          callback({
            data: response[0].result.data,
            error: null,
          });
        }
      } else {
        callback({
          data: null,
          error: "Error: " + xhr.status,
        });
      }
    };

    xhr.onerror = () => {
      console.error("Request failed due to a network error.");
      callback({
        data: null,
        error: "Network error.",
      });
    };

    if (isMutation) {
      xhr.send(JSON.stringify(input));
    } else {
      xhr.send();
    }
  }
}

module.exports = PlantFriendApi;
