package core

import (
	"fmt"
	"net/http"

	"encoding/json"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/httputils"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
)

func CreateLink(project_id int64, link_conf domain.LinkConf, is_operation bool) (link_info domain.LinkInfo, res statuscode.Response) {
	res = link_conf.CheckFeasibility()
	if !res.IsSuccess() {
		return link_info, res
	}

	params := map[string]any{"mode": map[bool]string{true: "operation", false: "design"}[is_operation]}
	url := fmt.Sprintf("%s/%s/project/%d/link?%s", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id, httputils.QueryParams(params))

	data, res := httputils.HttpClient(http.MethodPost, url, []byte(link_conf.String()))
	if !res.IsSuccess() {
		return link_info, res
	}

	res = link_info.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return link_info, res
	}

	return link_info, statuscode.StatusOK(link_info)
}

func UpdateLink(project_id int64, link_id int64, link_conf domain.LinkConf, is_operation bool) (link_info domain.LinkInfo, res statuscode.Response) {
	link, res := GetLink(project_id, link_id, is_operation)
	if !res.IsSuccess() {
		return link_info, res
	}

	link.LinkConf = link_conf
	params := map[string]any{"mode": map[bool]string{true: "operation", false: "design"}[is_operation]}
	url := fmt.Sprintf("%s/%s/project/%d/link?%s", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id, httputils.QueryParams(params))

	data, res := httputils.HttpClient(http.MethodPut, url, []byte(link.String()))
	if !res.IsSuccess() {
		return link_info, res
	}

	res = link_info.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return link_info, res
	}

	return link_info, statuscode.StatusOK(link_info)
}

func GetLink(project_id int64, link_id int64, is_operation bool) (link domain.Link, res statuscode.Response) {
	params := map[string]any{"mode": map[bool]string{true: "operation", false: "design"}[is_operation]}
	url := fmt.Sprintf("%s/%s/project/%d/link/%d?%s", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id, link_id, httputils.QueryParams(params))

	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return link, res
	}

	res = link.UnmarshalJSONData(data)
	if !res.IsSuccess() {
		return link, res
	}

	return link, statuscode.StatusOK(link)
}

func GetLinks(project_id int64, is_operation bool) (links []domain.Link, res statuscode.Response) {
	params := map[string]any{"mode": map[bool]string{true: "operation", false: "design"}[is_operation]}
	url := fmt.Sprintf("%s/%s/project/%d/links?%s", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id, httputils.QueryParams(params))

	data, res := httputils.HttpClient(http.MethodGet, url, nil)
	if !res.IsSuccess() {
		return links, res
	}

	var actLinks domain.Links
	err := json.Unmarshal(data, &actLinks)
	if err != nil { // Check for error instead of res.IsSuccess()
		return links, statuscode.StatusBadRequest(err.Error(), 0)
	}
	links = actLinks.Links
	return links, statuscode.StatusOK(links)
}

func DeleteLink(project_id, link_id int64, is_operation bool) (res statuscode.Response) {
	params := map[string]any{"mode": map[bool]string{true: "operation", false: "design"}[is_operation]}
	url := fmt.Sprintf("%s/%s/project/%d/link/%d?%s", GetCogsworthHttpEndpoint(), GetCogsworthAPIPathPrefix(), project_id, link_id, httputils.QueryParams(params))
	_, res = httputils.HttpClient(http.MethodDelete, url, nil)
	return res
}

func DeleteLinks(project_id int64, link_ids []int64, is_operation bool) (res statuscode.Response) {
	for _, link_id := range link_ids {
		res = DeleteLink(project_id, link_id, is_operation)
		if !res.IsSuccess() {
			return res
		}
	}
	return statuscode.StatusOK(nil)
}

func DeleteAllLinks(project_id int64, is_operation bool) (res statuscode.Response) {
	links, res := GetLinks(project_id, is_operation)
	if !res.IsSuccess() {
		return res
	}

	link_ids := make([]int64, 0, len(links))
	for _, link := range links {
		link_ids = append(link_ids, int64(link.Id))
	}

	res = DeleteLinks(project_id, link_ids, is_operation)
	if !res.IsSuccess() {
		return res
	}

	return statuscode.StatusOK(nil)
}
