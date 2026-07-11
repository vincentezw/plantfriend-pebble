module.exports = [
  {
    "type": "heading",
    "defaultValue": "Plant friend Settings"
  },
  {
    "type": "text",
    "defaultValue": "Enter your configuration details"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Access Configuration"
      },
      {
        "type": "input",
        "messageKey": "Token",
        "label": "Long-Lived Access Token",
        "defaultValue": "",
        "attributes": {
          "placeholder": "abcdef1234567890abcdef1234567890abcdef12"
        }
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
