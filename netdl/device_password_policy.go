package netdl

type PasswordPolicySetting struct {
	MinPasswordLength       *int  `json:"minPasswordLength,omitempty" validate:"required,min=4,max=63" comment:"Minimum Password Length"`
	RequireDigit            *bool `json:"requireDigit,omitempty" validate:"-" comment:"Must contain at least one digit (0-9)"`
	RequireUppercase        *bool `json:"requireUppercase,omitempty" validate:"-" comment:"Must contain at least one uppercase letter (A-Z)"`
	RequireLowercase        *bool `json:"requireLowercase,omitempty" validate:"-" comment:"Must contain at least one lowercase letter (a-z)"`
	RequireSpecialChar      *bool `json:"requireSpecialChar,omitempty" validate:"-" comment:"Must contain at least one special character (0|~!@#$%^&*._)"`
	MaxPasswordLifetimeDays *int  `json:"maxPasswordLifetimeDays,omitempty" validate:"min=0,max=365" comment:"Maximum Password Lifetime (days), 0 means no expiration"`
}
