
## ✅ Purpose
Trust Endorsement CORIM file defines **trust rules** for **Manticore CORIM Attestation**.  
It ensures firmware integrity by mapping **firmware versions** and **SVN (Security Version Number)** to a **TCB status** (`UpToDate` or `OutOfDate`).


## ✅ Structure Overview (Hierarchy)

``` 
corim-id
└── tag-identity
├── id
└── version
└── triples
└── conditional-endorsement-series [array]
├── statefulenv
│   ├── environment (layer)
│   └── measurements
│       └── svn 
│           • Enables selection of series that should apply.
│           • Default = min-value '0' → applies for svn ≥ 0.
└── series [array of rules]
    [
        {
            "selection_rule": "If firmware version selection matches exactly",
            "addition_action": "TCB status = UpToDate"
        },
        {
            "selection_rule": "Else if the provided firmware version selection is matching then it is no more available in the fleet",
            "addition_action": "TCB status = OutOfDate"
        },
        {
            "selection_rule": "Else if SVN >= svn_value",
            "addition_action": "TCB status = UpToDate"
        },
        {
            "selection_rule": "Else if SVN >= 0",
            "addition_action": "TCB status = OutOfDate"
        }
]

├── selection (version or svn condition)
└── addition (tcb-status)
```

### ✅ Trust Endorsement Logic
```
 "conditional-endorsement-series": [
    {
        "statefulenv": {
            "environment": {
                "class": {
                    "layer": Layer(0 and 1)
                }
            },
            "measurements": [
                {
                    "value": {
                        "svn": {
                            "type": "min-value", 
                            "value": 0          // Default baseline SVN (0 means allow all SVN ≥ 0)
                        }
                    }
                }
            ]
        },
        "series": [
            "selection": [
                    {
                        "value": {
                            "version": {
                                "value": fw_version,
                                "scheme": 0
                            }
                        }
                    }
                ],
                "addition": [
                    {
                        "value": {
                            "tcb-status": "UpToDate" // If version matches, mark as UpToDate
                        }
                    }
                ]
            },
            {
            "selection": [
                    {
                        "value": {
                            "version": {
                                "value": fw_version,
                                "scheme": 0
                            }
                        }
                    }
                ],
                "addition": [
                    {
                        "value": {
                            "tcb-status": "OutOfDate" // Mark the firmware version as OutOfDate, if the firmware is no longer available in the fleet.
                        }
                    }
                ]
            },
            {
                "selection": [
                    {
                        "value": {
                            "svn": {
                                "type": "min-value",  
                                "value": svn_value    // Minimum SVN required for UpToDate
                            }
                        }
                    }
                ],
                "addition": [
                    {
                        "value": {
                            "tcb-status": "UpToDate" // If SVN ≥ svn_value, mark as UpToDate
                        }
                    }
                ]
            },
            {
                "selection": [
                    {
                        "value": {
                            "svn": {
                                "type": "min-value", 
                                "value": 0    // SVN selection for OutofDate Criteria
                            }
                        }
                    }
                ],
                "addition": [
                    {
                        "value": {
                            "tcb-status": "OutOfDate" 
                        }
                    }
                ]
            }
        ]
    }
]        
```