package macutility

import "strings"

func chunkStr(str string, size int) []string {
	var res []string
	slice := []rune(str)

	if len(slice) == 0 || size <= 0 {
		return res
	}

	length := len(slice)
	if size == 1 || size >= length {
		for _, v := range slice {
			res = append(res, string(v))
		}
		return res
	}

	// divide slice equally
	divideNum := length/size + 1
	for i := 0; i < divideNum; i++ {
		if i == divideNum-1 {
			if len(slice[i*size:]) > 0 {
				res = append(res, string(slice[i*size:]))
			}
		} else {
			res = append(res, string(slice[i*size:(i+1)*size]))
		}
	}

	return res
}

func withColons(i string) string {
	p := chunkStr(i, 2)
	return strings.Join(p, ":")
}

func transLetterCase(s string, letterCase string) string {
	if letterCase == UpperCase {
		return strings.ToUpper(s)
	} else {
		return strings.ToLower(s)
	}
}
