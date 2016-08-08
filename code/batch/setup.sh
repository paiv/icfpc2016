#!/bin/sh

virtualenv .env
. activate

pip install unirest svgwrite
